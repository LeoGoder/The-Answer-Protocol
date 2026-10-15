#include "game_world.hpp"
#include "debug/debug.hpp"
#include "game_state.hpp"
#include "tcpip.h"
#include "helper.hpp"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <tyra>
#include <loadfile.h>
extern "C" {
    #include <kernel.h>
    #include <ps2ip.h>
    #include "netman.h"
}

static int ethGetNetIFLinkStatus(void) {
    return NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0)
           == NETMAN_NETIF_ETH_LINK_STATE_UP;
}

static void EthStatusCheckCb(s32 alarm_id, u16 time, void *common) {
    iWakeupThread(*(int *)common);
}

static int ethWaitValidNetIFLinkState(void) {
    int tid = GetThreadId();
    for (int retry = 0; ethGetNetIFLinkStatus() == 0; retry++) {
        SetAlarm(1000 * 16, &EthStatusCheckCb, &tid);
        SleepThread();
        if (retry >= 10) return -1;   // ~10 s
    }
    return 0;
}

static int ethApplyNetIFConfig(int mode) {
    static int CurrentMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;
    int result = 0;
    if (CurrentMode != mode) {
        if ((result = NetManSetLinkMode(mode)) == 0) CurrentMode = mode;
    }
    return result;
}

// libnetman keeps its "RPC client already initialised" flag in a static
// variable. On this setup that byte is already non-zero before the first
// NetManInit (it sits right before libmc's DMA'd `endParameter` buffer, which
// is only 8-byte aligned), so NetManInit silently skips the client init (no
// "NetMan" sema, no Tx/Rx thread) and every later netman RPC hangs forever.
// The flag is static, so find it by decoding the lui/lbu pairs at the start of
// NetManInit (order: netman flag, RPC server flag, RPC client flag) and clear it.
static void clearNetManClientFlag(void) {
    const uint32_t *code = (const uint32_t *)(void *)&NetManInit;
    uint32_t hi[32] = {0};
    bool have[32] = {false};
    int n_found = 0;
    for (int i = 0; i < 0x60; i++) {
        uint32_t w = code[i];
        uint32_t op = w >> 26, rs = (w >> 21) & 31, rt = (w >> 16) & 31;
        if (op == 0x0f) {  // lui
            hi[rt] = (uint32_t)(w & 0xffff) << 16;
            have[rt] = true;
        } else if (op == 0x24 && have[rs]) {  // lbu
            volatile uint8_t *flag = (volatile uint8_t *)(hi[rs] + (int16_t)(w & 0xffff));
            if (++n_found == 3) {
                if (*flag) {
                    TYRA_LOG("netman client flag was ", (int)*flag, ", clearing it");
                    *flag = 0;
                }
                return;
            }
            have[rt] = false;
        }
    }
    TYRA_LOG("netman client flag: not found");
}

GameWorld::GameWorld(Tyra::Engine *engine, GameState *game_state) {
    int ret, mod_res;

    load_image(&font, engine, "test_font.png");
    this->engine = engine;
    this->game_state = game_state;
    // Init network driver
    ret = SifExecModuleBuffer(ps2dev9_irx, size_ps2dev9_irx, 0, nullptr, &mod_res);
    TYRA_LOG("DEV9 buffer: ret=", ret, ", mod_res=", mod_res);

    ret = SifExecModuleBuffer(netman_irx, size_netman_irx, 0, nullptr, &mod_res);
    TYRA_LOG("NETMAN buffer: ret=", ret, ", mod_res=", mod_res);

    ret = SifExecModuleBuffer(smap_irx, size_smap_irx, 0, nullptr, &mod_res);
    TYRA_LOG("SMAP buffer: ret=", ret, ", mod_res=", mod_res);

    clearNetManClientFlag();
    ret = NetManInit();
    TYRA_LOG("NetManInit: res=", ret);
    ret = ethApplyNetIFConfig(NETMAN_NETIF_ETH_LINK_MODE_AUTO);
    TYRA_LOG("link mode: ret=", ret);

    // PCSX2 "Sockets" mode: its virtual router is 192.0.2.1 and it only talks
    // to the PS2 at 192.0.2.100 (replies are addressed to that IP, so any other
    // address makes lwIP drop the SYN-ACK). Real PS2 / PCAP: use your LAN values.
    ip4_addr_t ip, nm, gw;
    IP4_ADDR(&ip, 192, 0, 2, 100);
    IP4_ADDR(&nm, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 0, 2, 1);
    // ip4_addr_t ip, nm, gw;
    // IP4_ADDR(&ip, 192, 168, 1, 11);
    // IP4_ADDR(&nm, 255, 255, 255, 0);
    // IP4_ADDR(&gw, 192, 168, 1, 254);

    TYRA_LOG("ps2ipInit: start");
    ret = ps2ipInit(&ip, &nm, &gw);
    TYRA_LOG("ps2ipInit: ret=", ret);
    ret = ethWaitValidNetIFLinkState();
    TYRA_LOG("link wait: ret=", ret);
    memset(&this->server, 0, sizeof this->server);
    // Init struct
    this->server.sin_len = sizeof this->server;
    this->server.sin_family = AF_INET;
    this->server.sin_port = htons(SERVER_PORT);
    this->server.sin_addr.s_addr = inet_addr(SERVER_IP);
    this->fd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    TYRA_LOG("socket: fd=", this->fd);
}

GameWorld::~GameWorld() {
    this->engine->renderer.getTextureRepository().freeBySprite(font);
    if (this->fd >= 0) lwip_close(this->fd);
}

void GameWorld::update() {

}

void GameWorld::draw() {
    switch (this->intern_scene) {
        case connection: {
            TYRA_LOG("DIAG connect(", SERVER_IP, ":", SERVER_PORT, "): start");
            clock_t t0 = clock();
            int cret = lwip_connect(this->fd, (struct sockaddr *)&this->server, sizeof this->server);
            int cerrno = errno;
            int so_err = 0;
            socklen_t so_len = sizeof so_err;
            lwip_getsockopt(this->fd, SOL_SOCKET, SO_ERROR, &so_err, &so_len);
            TYRA_LOG("DIAG connect: ret=", cret, " errno=", cerrno, " SO_ERROR=", so_err,
                     " elapsed_ms=", (int)((clock() - t0) * 1000 / CLOCKS_PER_SEC));
            if (cret == 0) {
                int one = 1;
                lwip_ioctl(this->fd, FIONBIO, &one);
                this->intern_scene = island_1;
            }
            else {
                TYRA_LOG("CONNECTION ERROR");
                lwip_close(this->fd);
                this->fd = -1;
                this->intern_scene = connection_error;
            }
            break;
        }
        case connection_error:
            draw_text(this->engine, "Connection issue !", &this->font, 0, 0, 1.0f);
            break;
        case island_1:
            draw_text(this->engine, "Connection working !", &this->font, 0, 0, 1.0f);
            break;
    }
}
