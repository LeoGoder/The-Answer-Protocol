# Tuto serveur TAP

## Lancer et se connecter

```bash
make run-server          # depuis la racine (ou : cd server && go run ./cmd/server)
nc localhost 4242        # dans un autre terminal
```

Le serveur répond `OK hello proto=1`, puis on tape une commande par ligne.

- Commandes insensibles à la casse, 1024 octets max par ligne.
- Une réponse par commande : `OK ...` ou `ERR <code> <MESSAGE>`.
- Objets / PNJ : `item.1`, `1` ou `Object 1` (pareil pour `npc.2`, `2`, `NPC 2`).

## Carte

```
 room.2 ─── room.3 ─── room.4
   │          │          │
 room.1 ─── room.5 ─── room.6
                         │
                       room.7 ─── room.8
```

| Salle | Contenu |
|---|---|
| room.1 | départ / réapparition, `npc.1` (dialogue) |
| room.2 | `item.1` (soin +20) |
| room.3 | `npc.2` (quêtes) |
| room.4 | `item.2` (soin +40) |
| room.5 | `item.4` |
| room.7 | `npc.3` (ennemi, 80 PV) |
| room.8 | `item.3` (quête 1) |

## Commandes

### Connexion et exploration

| Commande | Réponse | Erreurs |
|---|---|---|
| `CONNECT alice` | `OK connected` | `201 NAME_IN_USE`, `400` nom invalide |
| `LOOK` | `OK {"room":{...},"players":[...],"items":[...],"npcs":[...]}` | |
| `MOVE north` | `OK room=room.2` | `301 NO_EXIT` |
| `WHO` | `OK players=3` | |
| `QUIT` | `OK bye` | |

Nom : 1 à 32 lettres/chiffres/`_`, pas plus de 3 caractères identiques de suite, sans `admin`.

### Chat et groupes

| Commande | Réponse | Erreurs |
|---|---|---|
| `CHAT ROOM salut` | `OK` + `EVT ROOM CHAT alice salut` | |
| `CHAT GLOBAL salut` | `OK` + `EVT GLOBAL CHAT alice salut` | |
| `CHAT GROUP salut` | `OK` + `EVT GROUP CHAT alice salut` | `401 NOT_IN_GROUP` |
| `GROUP CREATE` | `OK group=group.1` | `402 ALREADY_IN_GROUP` |
| `GROUP INVITE bob` | `OK` | `401`, `402`, `404 PLAYER_NOT_FOUND` |
| `GROUP JOIN alice` ou `GROUP JOIN group.1` | `OK group=group.1` | `403 NOT_INVITED`, `402`, `404` |
| `GROUP LEAVE` | `OK` | `401 NOT_IN_GROUP` |

### Objets

| Commande | Réponse | Erreurs |
|---|---|---|
| `TAKE item.1` | `OK taken=item.1` | `404 ITEM_NOT_FOUND` |
| `DROP item.1` | `OK dropped=item.1` | `404 ITEM_NOT_IN_INVENTORY` |
| `INVENTORY` | `OK ["item.1"]` | |
| `USE item.1` | `OK {"used":"item.1","hp":100,"max_hp":100}` | `404`, `408 ITEM_NOT_USABLE` |

### PNJ et combat

| Commande | Réponse | Erreurs |
|---|---|---|
| `TALK npc.1` | `OK Hello, I am NPC 1.` | `404 NPC_NOT_FOUND` |
| `ATTACK npc.3` | `OK {"attacker":"alice","target":"npc.3","attacker_hp":82,"target_hp":68,"damage":12,"damage_taken":18,"status":"combat"}` | `404`, `405 NPC_NOT_HOSTILE` |
| `STATUS` | `OK {"hp":82,"max_hp":100,"status":"combat","attack":10,"defense":2,"target":"npc.3"}` | |

Combat :
- 1 `ATTACK` = 1 tour : on frappe, puis l'ennemi riposte.
- `status` : `combat`, `victory` ou `defeated`.
- Mort : retour en room.1 avec la moitié des PV.
- `MOVE` quitte le combat. Un ennemi vaincu revient après 30 s.

### Quêtes

| Commande | Réponse | Erreurs |
|---|---|---|
| `QUEST npc.2` | `OK {"quest_id":"quest.1",...,"status":"active","progress":"0/1"}` | `404`, `406 NO_QUEST_AVAILABLE` |
| `QUESTS` | `OK [{"quest_id":"quest.1","status":"completed"},...]` | |

`QUEST npc.2` (en room.3) sert à **prendre, suivre et rendre** une quête. `TALK` ne valide rien.

| Quête | Objectif | Récompense |
|---|---|---|
| `quest.1` | rapporter `item.3` à `npc.2` | +10 PV max |
| `quest.2` | vaincre `npc.3` (quête prise **avant** le combat) | +3 attaque |

```
QUEST npc.2      room.3 : prendre quest.1
TAKE item.3      room.8
QUEST npc.2      room.3 : rendre quest.1
QUEST npc.2      room.3 : prendre quest.2
ATTACK npc.3     room.7 : jusqu'à "victory"
QUEST npc.2      room.3 : rendre quest.2
```

## Événements

| Événement | Destinataires |
|---|---|
| `EVT ROOM PRESENCE ENTER alice` / `LEAVE alice` | la salle |
| `EVT ROOM CHAT alice ...` | la salle |
| `EVT GLOBAL CHAT alice ...` | tout le monde |
| `EVT GROUP CHAT alice ...` | le groupe |
| `EVT GROUP INVITE alice` | le joueur invité |
| `EVT GROUP JOIN bob` / `LEAVE bob` | le groupe |
| `EVT STATS players=3` | tout le monde |
| `EVT ROOM COMBAT {...}` | la salle, sauf l'attaquant |

## Erreurs générales

| Erreur | Cause |
|---|---|
| `202 NOT_AUTHENTICATED` | commande avant `CONNECT` |
| `400 BAD_REQUEST` | mauvais arguments, caractères invalides |
| `400 UNKNOWN_COMMAND` | commande inconnue |
| `400 LINE_TOO_LONG` | ligne > 1024 octets |
| `900 CONNECTION_FAILED` | serveur plein (64 connexions) |
