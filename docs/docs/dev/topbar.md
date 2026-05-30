---
sidebar_position: 2
---

# Top Bar de l'éditeur

La top bar est le bandeau horizontal affiché en haut de l'éditeur. Elle fournit un menu principal pour l'application, avec des menus déroulants et des actions associées à chaque entrée.

Elle est implémentée dans `src/editor/ui/top_bar.hpp` et `src/editor/ui/top_bar.cpp`.

## Rôle

La top bar sert à :

- afficher les menus principaux de l'éditeur
- ouvrir et fermer les listes déroulantes
- déclencher des actions via des callbacks `on_click`
- bloquer temporairement les interactions du graphe quand un menu est ouvert

## Structure des données

L'API publique repose sur trois structures.

### `MenuItem`

Représente une entrée cliquable dans un menu.

```cpp
struct MenuItem {
    std::string label;
    std::function<void()> on_click;
};
```

- `label` : texte affiché dans la liste déroulante
- `on_click` : callback exécuté lors du clic sur l'entrée

### `Menu`

Représente un menu de la barre supérieure.

```cpp
struct Menu {
    std::string label;
    std::vector<MenuItem> items;
};
```

- `label` : libellé visible dans la barre
- `items` : ensemble des actions disponibles dans le menu

### `TopBarStyle`

Décrit l'apparence de la top bar.

Les champs principaux sont :

- `bar_height` : hauteur du bandeau principal
- `item_height` : hauteur de chaque ligne du menu déroulant
- `horizontal_padding` : marge horizontale dans les onglets
- `item_padding` : marge horizontale dans les items
- `minimum_menu_width` : largeur minimale d'un menu
- `font_size` : taille du texte
- couleurs de fond, de survol, de texte et de bordure

## Comportement

La classe `TopBar` gère l'état interactif du bandeau.

### Construction

```cpp
TopBar(std::vector<Menu> menus, TopBarStyle style = {});
```

Le constructeur reçoit la liste complète des menus et un style optionnel.

### Mise à jour

```cpp
void Update();
```

`Update()` lit la position de la souris et l'état du clic gauche pour :

- détecter le menu survolé
- détecter l'item survolé dans un menu ouvert
- ouvrir ou fermer un menu
- appeler le callback de l'item cliqué

### Dessin

```cpp
void Draw() const;
```

`Draw()` dessine :

- la barre principale
- les onglets des menus
- le menu déroulant actif
- les survols visuels sur le menu ou l'item actif

### Blocage de l'entrée du graphe

```cpp
bool BlocksGraphInput() const noexcept;
```

Cette méthode indique si la top bar doit empêcher les interactions avec le graphe.

Elle retourne `true` lorsqu'un menu est ouvert ou qu'un élément de la top bar est sous la souris.

## Règles de placement

- chaque menu occupe une largeur calculée à partir du texte et des marges
- la largeur minimale est contrôlée par `TopBarStyle::minimum_menu_width`
- le menu déroulant utilise la même largeur que l'onglet
- si le menu serait dessiné en dehors de la fenêtre, il est recentré vers la gauche pour rester visible

## Exemple d'utilisation

```cpp
using editor_ui::Menu;
using editor_ui::MenuItem;
using editor_ui::TopBar;

TopBar top_bar({
    {"File", {
        {"New", [] { /* créer un nouveau projet */ }},
        {"Open", [] { /* ouvrir un projet */ }},
    }},
    {"Edit", {
        {"Undo", [] { /* annuler la dernière action */ }},
    }},
});

// Dans la boucle principale
top_bar.Update();
top_bar.Draw();

if (top_bar.BlocksGraphInput()) {
    // Désactiver les interactions du graphe pendant l'ouverture du menu
}
```

## Points importants

- `SetMenus()` remplace la liste complète des menus et ferme le menu en cours
- `CloseMenu()` réinitialise l'état de survol de l'item actif
- `OpenMenu()` vérifie les bornes avant d'ouvrir un menu
- `ActivateItem()` ignore silencieusement les indices invalides

## Limites actuelles

- la top bar dépend de `raylib` pour la lecture de la souris et le rendu
- les tests unitaires les plus robustes sont donc centrés sur l'API publique et l'état interne observable
- les tests d'interaction complète nécessiteraient une couche d'abstraction supplémentaire pour simuler les entrées utilisateur
