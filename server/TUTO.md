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

### Les PNJ en bref

Il y a 3 PNJ, chacun a **un seul rôle** :

| PNJ | Où | Rôle | Commande utile |
|---|---|---|---|
| `npc.1` | room.1 | il parle, c'est tout | `TALK npc.1` |
| `npc.2` | room.3 | il donne les quêtes | `QUEST npc.2` |
| `npc.3` | room.7 | ennemi, on le tape | `ATTACK npc.3` |

Il faut être **dans la même salle** que le PNJ, sinon `ERR 404 NPC_NOT_FOUND`.

### Les quêtes en bref

**Une seule commande pour tout : `QUEST npc.2`** (dans room.3).
On la tape plusieurs fois, et la réponse change selon où on en est :

| Situation | Ce que `QUEST npc.2` fait | `status` |
|---|---|---|
| Je n'ai pas de quête | il me **donne** une quête | `active`, `0/1` |
| J'ai la quête, mais pas encore fait l'objectif | il me **redonne la même quête** (rappel) | `active`, `0/1` |
| J'ai la quête et j'ai fait l'objectif | il **termine** la quête + récompense | `completed` |
| Tout est fini | rien à donner | `ERR 406 NO_QUEST_AVAILABLE` |

> **« Il me redonne la même quête et ne me donne pas l'autre ? »**
> C'est normal : la quête 1 n'est pas finie. Il faut **d'abord faire l'objectif**,
> revenir, retaper `QUEST npc.2` → elle est terminée → retaper `QUEST npc.2` → quête 2.

- `TALK` ne sert **pas** aux quêtes.
- `QUESTS` (avec un S) affiche juste où on en est. `1/1` = objectif fait, il reste à revenir le dire à `npc.2`.

### Les 2 quêtes

| Quête | Objectif | Récompense |
|---|---|---|
| `quest.1` | avoir `item.3` (room.8) dans son inventaire, puis revenir voir `npc.2` | +10 PV max |
| `quest.2` | tuer `npc.3` (room.7), puis revenir voir `npc.2` | +3 attaque |

Attention :
- quest.1 : ne pas `DROP` l'objet avant de revenir.
- quest.2 : prendre la quête **avant** de tuer `npc.3`, sinon ça ne compte pas.

### Parcours complet (à copier)

```
QUEST npc.2      room.3 : je prends quest.1            -> active 0/1
TAKE item.3      room.8 : je ramasse l'objet
QUEST npc.2      room.3 : je rends quest.1             -> completed
QUEST npc.2      room.3 : je prends quest.2            -> active 0/1
ATTACK npc.3     room.7 : je répète jusqu'à "victory"
QUEST npc.2      room.3 : je rends quest.2             -> completed
QUEST npc.2      room.3 : plus rien                    -> ERR 406 NO_QUEST_AVAILABLE
```

Chemin : room.3 → room.5 → room.6 → room.7 → room.8 (et retour pareil).

> Le monde, les PNJ et les quêtes actuels sont **temporaires** (noms « Room 1 », « Object 3 »...) :
> ce sont des cases à remplir pour le game design. Ils sont dans `server/data/world.json`.

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
