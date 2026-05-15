# CMake Scripts

Ce dossier contient les scripts CMake pour la gestion du projet.

## CPM.cmake

CPM (CMake Package Manager) est utilisé pour gérer les dépendances du projet.

### Ajouter une dépendance

Dans le fichier `CMakeLists.txt` racine, décommentez et ajoutez vos dépendances :

```cmake
CPMAddPackage(
    NAME nom_package
    GITHUB_REPOSITORY owner/repo
    GIT_TAG version
)
```

### Exemples de bibliothèques populaires

```cmake
# Formatting
CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 10.2.1
)

# Logging
CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    GIT_TAG v1.13.0
)

# JSON
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.11.3
)

# CLI parsing
CPMAddPackage(
    NAME cxxopts
    GITHUB_REPOSITORY jarro2783/cxxopts
    GIT_TAG v3.1.1
)

# Testing
CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    GIT_TAG v3.5.2
)
```

### Utiliser une dépendance dans un module

Après avoir ajouté une dépendance avec CPM, linkez-la dans votre module :

```cmake
target_link_libraries(mon_module PRIVATE fmt::fmt)
target_link_libraries(mon_module PRIVATE spdlog::spdlog)
target_link_libraries(mon_module PRIVATE nlohmann_json::nlohmann_json)
```

### Cache des dépendances

CPM met en cache les dépendances dans `~/.cache/CPM` pour éviter les téléchargements répétés.

Pour forcer le re-téléchargement :
```bash
rm -rf ~/.cache/CPM
```
