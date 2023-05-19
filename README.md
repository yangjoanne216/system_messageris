# **messagerie instantanée** *（Serveur de chat multi-clients en C）*

## **Presentation**

Ce logiciel offre une simulation de salon de discussion de groupe. Tous les clients ont leur propre composée d'un identifiant unique et d'un nom individuel. Ce nom est déterminé par le premier message que chaque utilisateur envoie au serveur après avoir établi une connexion. Lorsque ctrl +c quitte le serveur, les messages envoyés par les clients qui sont encore en ligne à ce moment-là sont écrits dans messages.txt.

## **Utilisation**

### **1.Compilation**
Compilation du serveur
```
gcc serveur.c -pthread -o serveur
```
Compilation du client
``` 
gcc client.c -pthread -o client
```
### **2.Execution**
Exécution du serveur
```
./serveur

```
Exécution d'un client
```
./client
```

### **3.Command to exit the program（important）**
### Quitter le client : `quit` 
### Quitter le serveur : `ctrl + c`
### Le client passe en mode édit_only[^6] : `ctrl+c`
notre recommandation est de 'ctrl+c' directement dans le serveur et tous les clients seront fermés automatiquement [^2]. 

## Note
[^1].*Il est possible d'avoir plusieurs terminaux ouverts sur un même appareil informatique. Un terminal exécute le code du serveur, tandis qu'un ou plusieurs terminaux exécutent le code du client. Le rôle du serveur est de traiter les messages envoyés par le client. Ainsi, veuillez ne pas saisir d'informations sur le terminal du serveur, mais plutôt sur celui du client.*

[^2]*En ce qui concerne la quitter du programme, notre recommandation est de 'ctrl+c' directement dans le serveur et tous les clients seront fermés automatiquement. L'avantage de cette approche est que tous les messages envoyés par l'utilisateur sont enregistrés dans le fichier message.txt. Il est bien sûr possible de quitter un seul client en tapant 'quit' dans le client, mais les messages du client qui est sorti prématurément ne seront pas écrits dans messages.txt.*

[^3]*Afin de tester le code sans interférence avec l'instabilité du réseau, nous utilisons des sockets Unix, de sorte que la communication client-serveur ne peut s'effectuer que sur un seul appareil. Il n'est pas possible d'exécuter notre code sur plusieurs appareils.*


[^4]*Il est recommandé d'exécuter d'abord `./serveur`, puis d'exécuter `./client`, sinon la connexion client échouera facilement*

[^5]*Si vous ouvrez accidentellement deux serveurs en même temps, veuillez vous déconnecter des deux serveurs, puis vous connecter à un nouveau serveur*

[^6]mode édit_only :Messages sent by other clients are not displayed while the client is in edit_only_mode(Pour répondre aux exigences de l'étape 7 du thème du projet)

enter the edit_only_mode : `ctrl+c` , 

exit edit_only_mode : `sending a message`

[^7]Le serveur accepte jusqu'à NUM_MAX clients en même temps.Dans notre programme, ce nombre est 10

[^8]Ne pas envoyer plus de 100 caractères dans un seul message

## Capture d'écran du programme en cours de fonctionnement
Le terminal en haut à gauche exécute le serveur
![caputure](caputure.png "caputure d'écran")

## contact
Si vous avez des questions, veuillez contacter:

Yang YANG(yang.yang2@dauphine.eu)

Ningxin Ye(ningxin.ye@dauphine.eu)

Yiqing Chen(yiqing.chen@dauphine.eu)
