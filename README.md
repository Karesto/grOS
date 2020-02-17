Perfect OS.


Still Needs Developping.


Nouvelle source si jamais y'en a besoin : https://github.com/cfenollosa/os-tutorial



TODO : (si jamais vous vous ennuyez ^^)
 - pagination (compredre ce que c'est, comment ca marche et la réactiver)
 - table des interruptions (IDT)
 - prise en charge du clavier (après IDT)
 - gestion de la mémoire (tas, système de fichiers, basique quitte a l'améliorer ensuite)
 - gestion des processus
 - shell et redirections


Ahmed : Je vais essayer de m'occuper d'un de ces trucs, entre la pagination et/ou le shell.

HOP ! petit lien de ce qu'on doit faire globalement : 
https://wiki.osdev.org/Creating_an_Operating_System

Dorian: Bonjour
J'ai fait le tuto https://wiki.osdev.org/Bare_Bones, sans y comprendre grand
chose...

Pour compiler faut que tu te construise un compilateur croisé i864-elf
https://wiki.osdev.org/GCC_Cross-Compiler
Installe tous les packages sans trop te poser de questions...

De plus il te faudra 
- qemu (pour l'émulation), 
- grub2-common, grub-efi-amd64 pour créer l'iso, 
- mtools et xorriso (dépendances)

Enfin dans le makefile, remplace la ligne CPATH= par le chemin de ton compilateur

Exit