Un injecteur de DLL rapide et efficace pour Windows 10, codé en C. Ce petit projet fait suite à l'ancien injecteur publié en 2021 qui était absolument afreux. 

Celui-là propose plusieurs fonctionnalités : 

- Possibilité de charger une config et d'injecter la dll dans le processus de la précédente injection. Configuration qui est d'ailleurs enregistrée dans un fichier txt   
  lorsque l'injection réussie. 

- Injection utilisant différent moyens de création de threads.

- Thread Hijacking


Comment ça marche ? 

Avant de comprendre comment fonctionne l'injection de DLL dans un programme, je pense qu'il est évident de comprendre ce qu'est une DLL et son fonctionnement.

La DLL est un fichier qui contient de nombreuses fonctions et données utiles à un programme. Elle peut être utilisée par plusieurs programmes car son objectif vise à réutiliser du code et réduire l'espace disque utilisé. Par exemple, deux programmes peuvent utiliser la dll "msgbox.dll" pour afficher une boîte de dialogue spécifique à leur démarrage, leur évitant grossièremeent de réinventer la roue en créant une fonction "OpenMessage() dans leur code respectif.
Un processus peut ouvrir un ou plusieurs threads dans son espace de mémoire virtuel, lui permettant l'éxécution d'actions en simultanée et le partage de ressources.  

Injecter une DLL dans l'espace mémoire d'un programme permet donc d'en modifier son comportement et ses paramètres en modifiant ou utilisant le code d'une fonction. Cela est chose courante dans l'univers du game hacking ou il est possible d'utiliser une dll malveillante permettant par exemple au joueur de voir les textures et le modèles des joueurs ennemis à travers les murs. 

L'injection de DLL respecte ces étapes :  

1) Ouvrir un processus avec les droits requis en connaissant son PID (OpenProcess()) ;  
2) Créer un buffer et lui allouer de la mémoire dans le processus cible (VirtualAlloc()) ; 
3) Ecrire le chemin de la DLL dans ce buffer (WriteProcessMemory)) ; 
4) Utiliser le module Kernel32.dll pour appeller la fonction LoadLibraryA/W en obtenant son addresse (en utilisant la fonction GetProcAddress() en créant une variable de type 
   PTHREAD_START_ROUTINE). 
   LoadLibrary() prend en argument une librairie à charger ; 
5) Créer un thread exécuter LoadLibrary en asynchrone au sein du processus cible. 

Ci-dessus un schéma pour comprendre l'explication des étapes : 
![image](https://github.com/ulyssepmt/MEDUSA-Injector/assets/89702597/eb6544da-66e9-4012-9d7c-0e5721b9bc2c)

Sans surprise, certains programmes bloquent (on peut évoquer le terme de hooking de fonctions) l'utilisation de certaines fonctions issues de l'étape d'injection. Par exemple, l'utilisation de CreateRemoteThread() n'est même plus possible et il est donc nécéssaire de passer par des alternatives. 
J'ai dans mon cas utilisé RtlCreateUserThread et ZwCreateThreadEx qui sont deux fonctions de création de thread écrites dans le module Ntdll dont je récupère l'adresse (comme pour LoadLibrary). Pour des jeux comme CSGO (CS2.exe) et Valorant, je me suis rendu compte que cela ne servait plus à rien car l'un bloque la création de thread distant et l'autre n'autorise carrément pas l'allocation de mémoire.

Comme évoqué précédemment, un processus peut ouvrir des threads afin d'effectuer diverses tâches asynchrones. Une technique d'injection plus furtive consiste à injecter du code malveillant (capable de charger une dll en mémoire) dans un thread légitime sans avoir besoin de créer des threads distants. 
Cela peut être réaliser en suspendant le thread (SuspendThread()) en question (en récupérant son ID), puis en obtenant son contexte à l'aide de la fonction GetThreadContext().
Le contexte d'un thread correspond à son état (ensemble de registres, piles, etc) qui doit être suspendu pour y injecter un shellcode. En utilisant la structure de données CONTEXT, il est ainsi possible d'en modifier le pointeur d'instruction (rip) du context actuel du thread visé, qui doit pointer vers une adresse contenant le shellcode. Ce dernier contient : 
- le chemin de la DLL (adresse de chaîne de caractère) ; 
- l'adresse de LoadLibrary pour charger la DLL dans le processsus cible 
- la fonction RtlRestoreContext afin de renvoyer le pointeur d'instruction d'origine, comme si le thread n'avait pas été modifié. 

En un bref résumé, voici comment cela se déroule : 
Allocation de mémoire dans le processus cible pour 3 paramètres :  
- le chemin de la DLL ;
- le contexte du thread ;
- le shellcode.

Ces paramètres sont ensuite écrits dans l'espace mémoire du processus (comme pour une injection classique - à l'aide de WriteProcessMemory()). Le contexte du thread peut ainsi être modifié afin de pointer vers l'adresse du shellcode, qui sera exécuté par le thread détourné. Il ne reste plus qu'à relancer le thread avec le nouveau contexte dans son espace mémoire, en renvoyant l'adresse du pointeur d'instruction du contexte d'origine, comme si de rien n'était ! 

![hijack](https://github.com/ulyssepmt/MEDUSA-Injector/assets/89702597/b8d441b2-9b8f-438c-92fe-2e9c72d09eaa)

Il existe de nombreuses techniques d'injection furtives pour charger une DLL dans la mémoire d'un processus. Je pense que la meilleure reste selon moi l'injection manuelle (manual mapping) qui consiste à mapper les entête de la DLL (que l'essentiel à son chargement) et ses dépendances dans la mémoire du processus sans passer par LoadLibrary (qui laisse des traces) en utilisant GetProcAddress() souvent détectée par les AC. 


Je n'ai malheureusement pas assez de connaissances des entêtes PE d'une DLL et je n'ai pas souhaiter implémenter cette technique tout de suite dans l'injecteur. 



Vidéo démo : 



[Vidéo démo] 
