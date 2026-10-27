# Tuto serveur TAP

## Lancer et se connecter

```bash
make run-server          # depuis la racine (ou : cd server && go run ./cmd/server)
nc localhost 4242        # dans un autre terminal
```

Le serveur répond `OK hello proto=1`, puis on tape une commande par ligne.

- Commandes insensibles à la casse, 1024 octets max par ligne.
- Une réponse par commande : `OK ...` ou `ERR <code> <MESSAGE>`.
- Objets / PNJ : par **ID complet** (`item1`, `npc2`) ou par **nom** (`Farine`, `Chef Gusto`), majuscules ou pas.

## Carte : l'Archipel Culinaire

```
 room5 ─── room3 ──────────┐
   │         │             │
 room4 ─── room2 ─────── room6
   │         │
 room1 ───  hub  ─────── room7 (END)
```

| Salle | Nom | Contenu |
|---|---|---|
| `hub` | HUB - Le Restaurant et la Ferme | départ / réapparition, `npc1`, `npc2`, `item1` |
| `room1` | Ile 1 - Komolexos | `item2` |
| `room2` | Ile 2 - Jeetomal | `item3` |
| `room3` | Ile 3 - Smycka | `item4` |
| `room4` | Ile 4 - Vinterfint | `item5` |
| `room5` | Ile 5 - Fejka | `item6` |
| `room6` | Ile 6 - Kuggis | `npc3` (ennemi), `item7` |
| `room7` | Ile 7 (END) - Sanctuaire de Fugoumitak | `npc4` (boss), `item8` |

| Objet | Nom | Effet de `USE` |
|---|---|---|
| `item1` | Farine | – |
| `item2` | Tomate | soin +20 |
| `item3` | Boeuf | – |
| `item4` | Fromage | soin +35 |
| `item5` | Pomme de terre | – |
| `item6` | Oeuf | soin +25 |
| `item7` | Champignon | – |
| `item8` | Recette du Wellington | – |

## Commandes

### Connexion et exploration

| Commande | Réponse | Erreurs |
|---|---|---|
| `CONNECT alice` | `OK connected` | `201 NAME_IN_USE`, `400` nom invalide |
| `LOOK` | `OK {"room":{...},"players":[...],"items":[...],"npcs":[...],"names":{...}}` | |
| `MOVE north` | `OK room=room2` | `301 NO_EXIT` |
| `WHO` | `OK players=3` | |
| `ALIVE` | `OK Alive` | |
| `QUIT` | `OK bye` | |

Nom : 1 à 32 lettres/chiffres/`_`, pas plus de 3 caractères identiques de suite, sans `admin`.

`names` (ajout à la RFC) donne le nom de chaque ID visible : objets et PNJ de la salle, et salles voisines.
```
"names":{"item1":"Farine","npc1":"Apprenti Serveur","npc2":"Chef Gusto","room1":"Ile 1 - Komolexos",...}
```

### Chat et groupes

| Commande | Réponse | Erreurs |
|---|---|---|
| `CHAT ROOM salut` | `OK` + `EVT ROOM CHAT alice salut` | |
| `CHAT GLOBAL salut` | `OK` + `EVT GLOBAL CHAT alice salut` | |
| `CHAT GROUP salut` | `OK` + `EVT GROUP CHAT alice salut` | `401 NOT_IN_GROUP` |
| `GROUP CREATE` | `OK group=group.1` | `402 ALREADY_IN_GROUP` |
| `GROUP INVITE bob` | `OK` | `401`, `402`, `404 PLAYER_NOT_FOUND` |
| `GROUP JOIN alice` | `OK group=group.1` | `403 NOT_INVITED`, `402`, `404` |
| `GROUP LEAVE` | `OK` | `401 NOT_IN_GROUP` |

`GROUP JOIN` prend le **nom d'un joueur** du groupe (celui qui a invité), pas l'ID du groupe.

### Objets

| Commande | Réponse | Erreurs |
|---|---|---|
| `TAKE Farine` (ou `TAKE item1`) | `OK taken=item1` | `404 ITEM_NOT_FOUND` |
| `DROP item1` | `OK dropped=item1` | `404 ITEM_NOT_IN_INVENTORY` |
| `INVENTORY` | `OK ["item1"]` | |
| `USE Tomate` | `OK {"used":"item2","hp":100,"max_hp":100}` | `404`, `408 ITEM_NOT_USABLE` |

- Un objet utilisé (`USE`) ou rendu à une quête **réapparaît dans sa salle d'origine**.
- À la déconnexion, l'inventaire retourne aussi dans les salles d'origine.

### PNJ et combat

| Commande | Réponse | Erreurs |
|---|---|---|
| `TALK npc1` | `OK Bienvenue au HUB ! Nous n'avons que de la Farine ici.` | `404 NPC_NOT_FOUND` |
| `ATTACK npc3` | `OK {"attacker":"alice","target":"npc3","attacker_hp":88,"target_hp":50,"damage":10,"damage_taken":12,"status":"combat"}` | `404`, `405 NPC_NOT_HOSTILE` |
| `STATUS` | `OK {"hp":88,"max_hp":100,"status":"combat","attack":10,"defense":2,"target":"npc3"}` | |

Combat :
- 1 `ATTACK` = 1 tour : on frappe, puis l'ennemi riposte.
- Dégâts = attaque + aléatoire(0..4) - défense, minimum 1. Joueur au départ : 100 PV, attaque 10, défense 2.
- `status` : `combat`, `victory` ou `defeated`.
- Mort : retour au `hub` avec la moitié des PV max.
- `MOVE` quitte le combat. Un ennemi vaincu revient après 30 s.
- Plusieurs joueurs peuvent taper le même ennemi.

### Les PNJ en bref

| PNJ | Nom | Où | Rôle | Commande utile |
|---|---|---|---|---|
| `npc1` | Apprenti Serveur | hub | il parle, donne des conseils | `TALK npc1` |
| `npc2` | Chef Gusto | hub | il donne **toutes** les quêtes | `QUEST npc2` |
| `npc3` | Gardien Sauvage | room6 | ennemi : 60 PV, attaque 12, défense 1 | `ATTACK npc3` |
| `npc4` | Critique Supreme | room7 | boss : 110 PV, attaque 18, défense 2 | `ATTACK npc4` |

Il faut être **dans la même salle** que le PNJ, sinon `ERR 404 NPC_NOT_FOUND`.

### Les quêtes en bref

**Une seule commande pour tout : `QUEST npc2`** (dans le `hub`).
On la tape plusieurs fois, et la réponse change selon où on en est :

| Situation | Ce que `QUEST npc2` fait | `status` |
|---|---|---|
| Je n'ai pas de quête | il me **donne** la quête suivante | `active`, `0/1` |
| J'ai la quête, mais pas encore fait l'objectif | il me **redonne la même quête** (rappel) | `active`, `0/1` |
| J'ai la quête et j'ai fait l'objectif | il **termine** la quête + récompense | `completed` |
| Tout est fini | rien à donner | `ERR 406 NO_QUEST_AVAILABLE` |

> **« Il me redonne la même quête et ne me donne pas la suivante ? »**
> C'est normal : les quêtes sont **une chaîne**. Il faut faire l'objectif, revenir au hub,
> retaper `QUEST npc2` → terminée → retaper `QUEST npc2` → quête suivante.

- `TALK` ne sert **pas** aux quêtes.
- `QUESTS` (avec un S) affiche où on en est. `1/1` = objectif fait, il reste à revenir le dire à `npc2`.
- **fetch** : avoir l'objet dans l'inventaire en revenant. Ne pas le `DROP` avant !
- **defeat** : prendre la quête **avant** de tuer l'ennemi, sinon ça ne compte pas.

### Les 9 quêtes (dans l'ordre)

| Quête | Type | Objectif | Où | Récompense |
|---|---|---|---|---|
| `quest1` | fetch | rapporter la Tomate (`item2`) | room1 | +10 PV max, +2 attaque |
| `quest2` | fetch | rapporter le Boeuf (`item3`) | room2 | +10 PV max, +2 attaque |
| `quest3` | fetch | rapporter le Fromage (`item4`) | room3 | +15 PV max, +2 attaque |
| `quest4` | fetch | rapporter la Pomme de terre (`item5`) | room4 | +15 PV max, +2 attaque |
| `quest5` | fetch | rapporter l'Oeuf (`item6`) | room5 | +15 PV max, +3 attaque |
| `quest6` | defeat | vaincre le Gardien Sauvage (`npc3`) | room6 | +20 PV max, +4 attaque |
| `quest7` | fetch | rapporter le Champignon (`item7`) | room6 | +20 PV max, +4 attaque |
| `quest8` | defeat | vaincre le Critique Supreme (`npc4`) | room7 | +30 PV max, +5 attaque |
| `quest9` | fetch | rapporter la Recette du Wellington (`item8`) | room7 | +50 PV max, +10 attaque |

### Parcours complet (à copier)

Toujours le même rythme : `QUEST npc2` au hub → aller chercher → revenir → `QUEST npc2`.

```
hub   QUEST npc2       prendre quest1
room1 TAKE Tomate      (MOVE west)
hub   QUEST npc2       rendre quest1 -> completed
hub   QUEST npc2       prendre quest2
room2 TAKE Boeuf       (MOVE north)
hub   QUEST npc2       rendre quest2
...   quest3 a quest5 pareil : room3 Fromage, room4 Pomme de terre, room5 Oeuf
hub   QUEST npc2       prendre quest6
room6 ATTACK npc3      répéter jusqu'à "victory"   (hub -> room2 -> room6)
hub   QUEST npc2       rendre quest6, puis prendre quest7
room6 TAKE Champignon
hub   QUEST npc2       rendre quest7, puis prendre quest8
room7 ATTACK npc4      répéter jusqu'à "victory"   (MOVE east depuis le hub)
hub   QUEST npc2       rendre quest8, puis prendre quest9
room7 TAKE Recette du Wellington
hub   QUEST npc2       rendre quest9 -> fin du jeu
```

Astuce : garder Tomate / Fromage / Oeuf pour se soigner (`USE`) pendant les combats.

## Positions dans la salle (ajout à la RFC, pour le GUI)

| Commande | Réponse | Erreurs |
|---|---|---|
| `POSITION 120 840` | `OK` | `400 BAD_REQUEST` (pas des nombres, ou hors de 0..1000) |

- `x` et `y` vont de 0 à 1000 ; `(0,0)` = coin nord-ouest du sol.
- Le GUI envoie sa position **une fois en arrivant** dans une salle (et après `CONNECT`),
  puis seulement quand elle change.
- Un client qui a envoyé au moins un `POSITION` reçoit 24 fois par seconde :
  `EVT ROOM POSITION {"alice":[120,840],"bob":[500,500]}` (positions des joueurs de sa salle).
- Un client qui n'envoie jamais `POSITION` (le CLI) ne reçoit **aucun** de ces événements.

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
| `EVT ROOM POSITION {...}` | la salle, seulement les clients qui envoient `POSITION` |

## Erreurs générales

| Erreur | Cause |
|---|---|
| `202 NOT_AUTHENTICATED` | commande avant `CONNECT` |
| `400 BAD_REQUEST` | mauvais arguments, caractères invalides |
| `400 UNKNOWN_COMMAND` | commande inconnue |
| `400 LINE_TOO_LONG` | ligne > 1024 octets |
| `500 INTERNAL_ERROR` | ne devrait jamais arriver |
| `900 CONNECTION_FAILED` | serveur plein (64 connexions) |
