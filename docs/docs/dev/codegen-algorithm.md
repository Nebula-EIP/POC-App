---
sidebar_position: 2
---

# Pipeline de génération C++

Cette page décrit l’algorithme utilisé pour transformer un graphe Nebula en code C++ généré.

## Objectif

Le but est simple : prendre un graphe de nœuds connecté, vérifier qu’il est valide, ordonner ses dépendances, puis produire un fichier C++ compilable.

L’algorithme essaie de respecter deux priorités :

1. générer un code correct et compilable
2. garder un code lisible quand on veut exporter un exemple humainement compréhensible

## Vue d’ensemble

Le pipeline complet suit ces étapes :

1. validation du graphe
2. tri topologique des nœuds
3. génération des expressions et des instructions
4. gestion des blocs de contrôle (`if`, `while`, `for`)
5. option de pliage de constantes
6. export du résultat en fichier `.cpp`

## 1. Validation du graphe

Avant toute génération, le graphe est validé par `GraphValidator`.

### Ce que cette phase vérifie

- absence de cycle dans les dépendances
- compatibilité des types entre pins connectés
- présence des connexions obligatoires
- cohérence des entrées et sorties attendues par chaque nœud

### Pourquoi c’est important

Sans cette étape, le générateur pourrait produire un code incohérent, voire entrer dans une boucle infinie lors de l’analyse des dépendances.

## 2. Tri topologique

Une fois le graphe valide, les nœuds sont ordonnés avec un tri topologique de type Kahn.

### Rôle du tri topologique

- garantir qu’un nœud est généré après ses dépendances
- éviter de référencer une variable C++ avant sa déclaration
- stabiliser l’ordre d’émission du code

### Résultat

Le générateur reçoit une liste ordonnée de nœuds dans un ordre compatible avec l’émission séquentielle du C++.

## 3. Émission du code

`CodegenContext` transforme ensuite les nœuds en lignes C++.

### Types de nœuds gérés

- `Literal` : produit une constante C++
- `Operator` : produit une expression arithmétique, logique ou comparative
- `Print` : produit un `std::cout`
- `Condition` : produit un bloc `if` / `else`
- `Loop` : produit un bloc `while`
- `For` : produit un bloc `for`

### Principe d’émission

Le générateur parcourt les nœuds ordonnés et crée un symbole C++ par nœud, par exemple :

- `lit_0` pour une constante
- `tmp_3` pour un opérateur intermédiaire
- `result_0` pour un résultat visible dans la version avec sorties

## 4. Gestion des blocs de contrôle

Les nœuds de contrôle ont un traitement spécial.

### `Condition`

- lit la condition booléenne depuis l’entrée
- émet `if (...) {`
- émet les nœuds de la branche vraie
- émet éventuellement `else { ... }`

### `Loop`

- lit la condition booléenne depuis l’entrée
- émet `while (...) {`
- émet les nœuds du corps de boucle

### `For`

- lit l’initialisation, la condition et l’incrément
- émet `for (init; condition; increment) {`
- émet les nœuds du corps de boucle

### Indentation

Les instructions à l’intérieur des blocs sont émises avec un niveau d’indentation supplémentaire pour rester lisibles.

## 5. Pliage de constantes

Le générateur peut simplifier certaines expressions si toutes les entrées sont connues à la génération.

### Exemple

Si le graphe contient `3 + 4`, le générateur peut produire directement `7`.

### Mode configurable

Le pliage de constantes est optionnel :

- activé : code plus compact et plus optimisé
- désactivé : code plus fidèle visuellement au graphe d’origine

## 6. Export du fichier

`GraphExporter` prend le code généré et l’écrit dans `examples/output/`.

### Étapes de l’export

- validation de la requête d’export
- génération du contenu C++
- écriture du fichier sur disque

## Algo utilisés

- **DFS** : détection de cycles dans le graphe
- **Tri topologique de Kahn** : ordonnancement des nœuds
- **Traversal récursif** : émission des sous-arbres et des branches
- **Constant Folding** : calcul anticipé des expressions constantes

## Limites actuelles

Le système fonctionne bien pour les cas déjà supportés, mais il a encore des limites.

### Types supportés

- `int`
- `float` / `double`
- `bool`
- `string`
- `void` pour les flux de contrôle

### Types non supportés pour l’instant

- `char`
- `long`, `unsigned`, `size_t`
- tableaux et conteneurs (`std::vector`, etc.)
- structs et objets métier
- pointeurs et références
- types génériques ou templates utilisateur

### Limites fonctionnelles

- pas de `break` / `continue`
- pas de variables mutables de type SSA ou d’assignation avancée
- pas de pipeline IR intermédiaire
- pas d’optimisations globales avancées

## Pistes d’amélioration

Voici ce qu’on pourrait ajouter plus tard :

1. support de nouveaux types C++
2. vraie gestion des variables mutables et des affectations
3. nœud `for` dédié
4. `break` et `continue`
5. transformations intermédiaires avant émission C++
6. optimisations plus avancées comme la suppression de code mort globale
7. meilleure prise en charge des fonctions utilisateur et des scopes
8. génération de code encore plus structurée avec une IR interne

## Résumé

Le générateur suit donc une chaîne assez classique :

**validation → tri topologique → émission C++ → export**

Le cœur de l’algorithme est déjà solide pour les graphes simples, les opérateurs, l’affichage, les conditions et les boucles. Les prochaines étapes naturelles sont surtout l’extension des types, des contrôles de flux et des optimisations.