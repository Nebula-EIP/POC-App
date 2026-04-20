---
id: cmake-tool
---

# Guide Utilisateur - Outil de Compilation CMake

## Vue d'ensemble

L'outil de compilation CMake intégré à l'éditeur permet de compiler rapidement des fichiers C++ individuels avec des options personnalisables, sans avoir à gérer manuellement la configuration CMake.

## Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Démarrage rapide](#démarrage-rapide)
- [Interface utilisateur](#interface-utilisateur)
- [Options de compilation](#options-de-compilation)
- [Exemples d'utilisation](#exemples-dutilisation)
- [Dépannage](#dépannage)

## Démarrage rapide

### Compilation simple

1. Ouvrez un fichier C++ dans l'éditeur
2. Accédez au menu **Outils** → **Compiler avec CMake**
3. Cliquez sur **Compiler**
4. L'exécutable sera généré dans le répertoire de build

### Compilation et exécution

1. Ouvrez votre fichier source C++
2. Accédez au menu **Outils** → **Compiler et Exécuter**
3. Le fichier sera compilé puis exécuté automatiquement
4. La sortie s'affichera dans le terminal intégré

## Interface utilisateur

### Panneau de configuration de compilation

Lorsque vous lancez l'outil de compilation CMake, une fenêtre de configuration s'ouvre avec les sections suivantes :

#### 1. **Fichier source**
- **Chemin** : Chemin complet du fichier à compiler (automatiquement rempli avec le fichier actif)
- **Bouton Parcourir** : Permet de sélectionner un autre fichier source

#### 2. **Configuration générale**

##### Standard C++
Sélectionnez le standard C++ à utiliser :
- C++11 (ISO/IEC 14882:2011)
- C++14 (ISO/IEC 14882:2014)
- C++17 (ISO/IEC 14882:2017)
- C++20 (ISO/IEC 14882:2020)
- **C++23** (ISO/IEC 14882:2023) - *Par défaut*

##### Type de build
Choisissez le type de compilation :
- **Release** : Optimisé pour la performance (par défaut)
- **Debug** : Avec symboles de débogage, sans optimisation
- **RelWithDebInfo** : Optimisé avec informations de débogage
- **MinSizeRel** : Optimisé pour la taille du binaire

##### Nom de sortie
- **Champ texte** : Nom personnalisé pour l'exécutable
- Si laissé vide, utilise le nom du fichier source

#### 3. **Options avancées**

##### Drapeaux de compilation
Liste des drapeaux passés au compilateur :
- **Ajouter un drapeau** : Bouton `+` pour ajouter un nouveau drapeau
- **Exemples** : `-Wall`, `-Wextra`, `-pedantic`, `-O3`, etc.
- **Supprimer** : Cliquez sur `✖` à côté d'un drapeau pour le retirer

##### Drapeaux de liaison
Liste des options pour l'éditeur de liens :
- **Ajouter un drapeau** : Bouton `+` pour ajouter
- **Exemples** : `-lpthread`, `-lm`, `-static`, etc.

##### Répertoires d'inclusion
Chemins des fichiers d'en-tête supplémentaires :
- **Ajouter un chemin** : Bouton `+` pour ajouter
- **Parcourir** : Sélectionner un répertoire via le navigateur de fichiers
- **Exemples** : `/usr/local/include`, `./include`, `../external/headers`

##### Bibliothèques
Noms des bibliothèques à lier :
- **Ajouter une bibliothèque** : Bouton `+`
- **Exemples** : `pthread`, `m` (math), `boost_filesystem`
- **Note** : Ne pas inclure le préfixe `lib` ni l'extension

##### Définitions de préprocesseur
Macros et définitions pour le préprocesseur :
- **Ajouter une définition** : Bouton `+`
- **Format** : `NOM=valeur` ou simplement `NOM`
- **Exemples** :
  - `DEBUG_MODE=1`
  - `VERSION=2.5`
  - `ENABLE_LOGGING`

#### 4. **Options d'exécution**

##### Répertoire de build
- **Chemin** : Emplacement où CMake génère les fichiers de build
- **Par défaut** : `./cmake_build_<nom_fichier>`
- **Bouton Parcourir** : Choisir un répertoire personnalisé
- **Nettoyer le build** : Case à cocher pour supprimer le répertoire avant compilation

##### Mode verbeux
- **Case à cocher** : Active l'affichage détaillé de la compilation
- **Utilité** : Voir toutes les commandes CMake et du compilateur exécutées

##### Exécuter après compilation
- **Case à cocher** : Lance automatiquement l'exécutable après une compilation réussie
- **Terminal** : L'exécution s'affiche dans le terminal intégré

#### 5. **Actions**

##### Bouton "Compiler"
- Lance la compilation avec les options configurées
- **Icône** : 🔨 ou ⚙️
- **Raccourci** : `Ctrl+B` (configurable)

##### Bouton "Compiler et Exécuter"
- Équivalent à cocher "Exécuter après compilation" puis compiler
- **Icône** : ▶️
- **Raccourci** : `Ctrl+Shift+B` (configurable)

##### Bouton "Annuler"
- Ferme le panneau sans compiler
- **Raccourci** : `Échap`

##### Bouton "Réinitialiser"
- Restaure les paramètres par défaut
- Conserve uniquement le chemin du fichier source

## Options de compilation

### Standards C++ supportés

| Standard | Description | Fonctionnalités clés |
|----------|-------------|---------------------|
| C++11 | Premier standard moderne | Lambdas, auto, move semantics |
| C++14 | Améliorations mineures | Generic lambdas, return type deduction |
| C++17 | Évolution majeure | std::optional, std::filesystem, structured bindings |
| C++20 | Standard moderne | Concepts, ranges, coroutines, modules |
| C++23 | Dernier standard | std::expected, multidimensional subscript, ranges improvements |

### Types de build expliqués

#### Release (Production)
```
Optimisations : -O3
Symboles debug : Non
Usage : Version finale, performance maximale
Taille binaire : Moyenne
```

#### Debug (Développement)
```
Optimisations : -O0
Symboles debug : Complets (-g)
Usage : Développement, débogage avec GDB/LLDB
Taille binaire : Large
```

#### RelWithDebInfo (Profiling)
```
Optimisations : -O2
Symboles debug : Oui (-g)
Usage : Profiling, analyse de performance
Taille binaire : Large
```

#### MinSizeRel (Embarqué)
```
Optimisations : -Os (taille)
Symboles debug : Non
Usage : Systèmes embarqués, contraintes mémoire
Taille binaire : Minimale
```

### Drapeaux de compilation courants

#### Avertissements
```
-Wall          # Active les avertissements de base
-Wextra        # Avertissements supplémentaires
-Werror        # Traite les avertissements comme des erreurs
-pedantic      # Respect strict du standard
-Wshadow       # Avertit des variables cachées
-Wconversion   # Avertit des conversions implicites
```

#### Optimisations
```
-O0            # Aucune optimisation (debug)
-O1            # Optimisations basiques
-O2            # Optimisations standard (recommandé)
-O3            # Optimisations agressives
-Os            # Optimisation pour la taille
-Ofast         # -O3 + optimisations non conformes IEEE
-march=native  # Optimisations pour le CPU actuel
```

#### Débogage et analyse
```
-g             # Informations de débogage
-g3            # Informations de débogage maximales
-fsanitize=address        # AddressSanitizer (détection de fuites mémoire)
-fsanitize=thread         # ThreadSanitizer (race conditions)
-fsanitize=undefined      # UndefinedBehaviorSanitizer
-fno-omit-frame-pointer   # Conserve les pointeurs de frame (profiling)
```

### Drapeaux de liaison courants

```
-lpthread      # Bibliothèque POSIX threads
-lm            # Bibliothèque mathématique
-ldl           # Chargement dynamique
-static        # Liaison statique
-Wl,-rpath,.   # Chemin de recherche runtime
```

## Exemples d'utilisation

### Exemple 1 : Programme simple

**Contexte** : Compiler un fichier `hello.cpp` basique

**Configuration** :
- Standard C++ : C++17
- Type de build : Release
- Options avancées : Aucune

**Résultat** : Exécutable `hello` dans `./cmake_build_hello/bin/`

---

### Exemple 2 : Programme avec avertissements stricts

**Contexte** : Code de production nécessitant qualité maximale

**Configuration** :
- Standard C++ : C++20
- Type de build : Release
- Drapeaux de compilation :
  - `-Wall`
  - `-Wextra`
  - `-Werror`
  - `-pedantic`

**Résultat** : Compilation échoue si des avertissements sont présents

---

### Exemple 3 : Débogage avec sanitizer

**Contexte** : Recherche de fuites mémoire

**Configuration** :
- Standard C++ : C++17
- Type de build : Debug
- Drapeaux de compilation :
  - `-fsanitize=address`
  - `-fno-omit-frame-pointer`
  - `-g`
- Drapeaux de liaison :
  - `-fsanitize=address`
- Exécuter après compilation : ✓

**Résultat** : Programme s'exécute avec AddressSanitizer actif

---

### Exemple 4 : Utilisation de bibliothèques externes

**Contexte** : Programme utilisant Boost.Filesystem

**Configuration** :
- Standard C++ : C++17
- Type de build : Release
- Répertoires d'inclusion :
  - `/usr/local/include`
- Bibliothèques :
  - `boost_filesystem`
  - `boost_system`
- Drapeaux de liaison :
  - `-lpthread`

---

### Exemple 5 : Compilation conditionnelle

**Contexte** : Code avec sections DEBUG/RELEASE

**Configuration Debug** :
- Standard C++ : C++20
- Type de build : Debug
- Définitions :
  - `DEBUG_MODE=1`
  - `LOG_LEVEL=VERBOSE`
  - `ENABLE_ASSERTIONS`

**Configuration Release** :
- Standard C++ : C++20
- Type de build : Release
- Définitions :
  - `NDEBUG`
  - `LOG_LEVEL=ERROR`

**Code exemple** :
```cpp
#ifdef DEBUG_MODE
    std::cout << "Debug: Variable value = " << var << std::endl;
#endif
```

---

### Exemple 6 : Projet multi-fichiers avec en-têtes

**Structure** :
```
project/
├── src/
│   └── main.cpp
├── include/
│   ├── utils.hpp
│   └── config.hpp
└── lib/
    └── helper.cpp
```

**Configuration** :
- Standard C++ : C++23
- Type de build : Release
- Répertoires d'inclusion :
  - `./include`
  - `./lib`

**Note** : Pour plusieurs fichiers source, créez une bibliothèque statique ou utilisez un CMakeLists.txt complet

---

### Exemple 7 : Optimisation maximale

**Contexte** : Code critique en performance (calcul scientifique)

**Configuration** :
- Standard C++ : C++20
- Type de build : Release
- Drapeaux de compilation :
  - `-O3`
  - `-march=native`
  - `-flto` (Link Time Optimization)
  - `-funroll-loops`
- Drapeaux de liaison :
  - `-flto`

---

### Exemple 8 : Build minimal pour embarqué

**Contexte** : Système embarqué avec contraintes mémoire

**Configuration** :
- Standard C++ : C++17
- Type de build : MinSizeRel
- Drapeaux de compilation :
  - `-fno-exceptions`
  - `-fno-rtti`
  - `-ffunction-sections`
  - `-fdata-sections`
- Drapeaux de liaison :
  - `-Wl,--gc-sections`
  - `-static`

## Dépannage

### Problèmes courants

#### 1. "Fichier d'en-tête introuvable"

**Symptôme** :
```
fatal error: myheader.hpp: No such file or directory
```

**Solution** :
- Ajoutez le chemin dans **Répertoires d'inclusion**
- Vérifiez que le chemin est correct (absolu ou relatif au fichier source)

#### 2. "Référence indéfinie" lors de la liaison

**Symptôme** :
```
undefined reference to `pthread_create'
```

**Solution** :
- Ajoutez la bibliothèque manquante dans **Bibliothèques** (ex: `pthread`)
- Ou ajoutez le drapeau de liaison correspondant (ex: `-lpthread`)

#### 3. "Fonctionnalité C++ non supportée"

**Symptôme** :
```
error: 'std::filesystem' is not a namespace-name
```

**Solution** :
- Augmentez le **Standard C++** (ex: C++17 minimum pour std::filesystem)
- Vérifiez la compatibilité de votre compilateur

#### 4. Compilation réussie mais exécutable introuvable

**Symptôme** :
```
Compilation completed but executable not found
```

**Solution** :
- Vérifiez le **Répertoire de build**
- Essayez de cocher **Nettoyer le build** avant compilation
- Vérifiez les permissions du système de fichiers

#### 5. Erreurs de sanitizer lors de l'exécution

**Symptôme** :
```
AddressSanitizer: heap-use-after-free
```

**Solution** :
- Ce n'est pas une erreur de l'outil mais une erreur détectée dans votre code
- Utilisez les informations fournies pour localiser le bug
- Compilez avec `-g` pour obtenir des numéros de ligne précis

#### 6. Compilation lente

**Solution** :
- Ne cochez pas **Nettoyer le build** pour les compilations incrémentales
- Réduisez le niveau d'optimisation pendant le développement (utilisez Debug)
- Désactivez **Mode verbeux** sauf en cas de problème

#### 7. Message "CMake configuration failed"

**Symptôme** :
```
CMake Error: Could not create named generator
```

**Solution** :
- Vérifiez que CMake est installé (`cmake --version`)
- Vérifiez que le compilateur C++ est accessible
- Essayez de nettoyer le répertoire de build

## Raccourcis clavier

| Action | Raccourci | Description |
|--------|-----------|-------------|
| Compiler le fichier actif | `Ctrl+B` | Ouvre le panneau de compilation |
| Compiler et exécuter | `Ctrl+Shift+B` | Compile puis exécute |
| Arrêter la compilation | `Ctrl+C` | Arrête le processus en cours |
| Fermer le panneau | `Échap` | Ferme sans compiler |
| Naviguer entre champs | `Tab` | Passe au champ suivant |
| Activer/désactiver option | `Espace` | Pour les cases à cocher |

## Astuces et bonnes pratiques

### 1. Profils de compilation

Créez des profils réutilisables pour vos configurations courantes :
- **Dev** : C++20, Debug, -Wall -Wextra
- **Prod** : C++20, Release, -O3 -Wall -Werror
- **Profile** : C++20, RelWithDebInfo, -fno-omit-frame-pointer

### 2. Drapeaux recommandés par cas d'usage

**Développement quotidien** :
```
-Wall -Wextra -g
```

**Code de production** :
```
-Wall -Wextra -Werror -O3 -DNDEBUG
```

**Recherche de bugs** :
```
-Wall -Wextra -g -fsanitize=address -fsanitize=undefined
```

**Performance critique** :
```
-O3 -march=native -flto -DNDEBUG
```

### 3. Organisation des builds

- Utilisez des répertoires de build séparés pour Debug/Release
- Exemple : `./cmake_build_debug` et `./cmake_build_release`
- Ne versionnez pas les répertoires de build dans Git

### 4. Compilation incrémentale

- Ne nettoyez le build que si nécessaire
- CMake reconstruira uniquement les fichiers modifiés
- Gain de temps considérable sur les gros projets

### 5. Mode verbeux

- Activez-le pour comprendre ce qui se passe en cas d'erreur
- Désactivez-le pour les compilations rapides sans problème
- Utile pour déboguer les problèmes de liaison

## Limites actuelles

### Fichier source unique
L'outil est conçu pour compiler **un seul fichier source** à la fois. Pour des projets multi-fichiers complexes :
- Utilisez un `CMakeLists.txt` complet dans votre projet
- Ou créez une bibliothèque statique puis liez-la

### Pas de cache de configuration
Les paramètres ne sont pas sauvegardés entre les sessions. Pour réutiliser une configuration :
- Créez des profils personnalisés (fonctionnalité future)
- Ou utilisez l'outil CLI avec un script

### Compilation croisée
La compilation croisée (cross-compilation) n'est pas supportée via l'interface graphique. Utilisez CMake directement pour ces cas avancés.

## Support et ressources

### Documentation technique
- [CMake Official Documentation](https://cmake.org/documentation/)
- [GCC Compiler Options](https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html)
- [Clang Compiler Options](https://clang.llvm.org/docs/ClangCommandLineReference.html)

### Standards C++
- [C++17 Reference](https://en.cppreference.com/w/cpp/17)
- [C++20 Reference](https://en.cppreference.com/w/cpp/20)
- [C++23 Reference](https://en.cppreference.com/w/cpp/23)

### Outils de débogage
- [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [GDB Tutorial](https://www.gnu.org/software/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)

---

**Version** : 1.0
**Dernière mise à jour** : Mars 2026
**Auteur** : Équipe Nebula Editor
