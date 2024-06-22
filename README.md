# Multiple-Chat

![License](https://img.shields.io/badge/license-GNU-blue.svg)
![Version](https://img.shields.io/badge/version-1.0.0-brightgreen.svg)

Multiple-Chat è un software scritto in C che consente di realizzare una chat di gruppo tra più host.

## Sommario

- [Introduzione](#introduzione)
- [Caratteristiche](#caratteristiche)
- [Installazione e Uso](#installazione)
- [Avvertenze](#avvertenze)
- [Licenza](#licenza)
- [Contatti](#contatti)

## Introduzione

Multiple-Chat è stato realizzato per fornire un servizio di chat di gruppo. 

## Caratteristiche

- **Applicazione peer-to-peer 1**: Multiple-Chat può essere eseguita come server o client utilizzando lo stesso eseguibile.
- **Comandi**: Multiple-Chat mette a dispozione, dei comandi speciali come pulizia della cronologia e l'uscita dalla chat.
- **Log**: Alla fine di ogni connessione viene creato un file di log che riassume tutto quello che è accaduto nella chat.
- **Finestre**: Il client ha a disposizione 3 finestre sul proprio terminale(non visibili) una finestra dove poter scrivere il messaggio, una dove vengono visualizzati tutti i messaggi scritti dal client e infine la finestra dove vengono visualizzati tutti i messaggi ricevuti dagli altri client.
- **Ordine dei messaggi**: I messaggi vengono visualizzati secondo un ordine diagonale.

## Installazione e Uso

Istruzioni su come usare Multiple-Chat lato server:
```bash
git clone https://github.com/jim-bug/Multiple-Chat.git
cd Multiple-Chat
bash multiple_chat.sh -s
```
Istruzioni su come usare Multiple-Chat lato client:
```bash
git clone https://github.com/jim-bug/Multiple-Chat.git
cd Multiple-Chat
bash multiple_chat.sh -c IP PORT NAME
```

### Sostituisci

```IP```: Indirizzo IPv4 del server \
```PORT```: Porta del server \
```NAME```: Nome da utilizzare nella chat

### Note
- **Nome Client**: Il lunghezza del nome del client deve essere compresa tra 1 e 100.
- **Lunghezza messaggio**: La lunghezza del messaggio deve deve compresa tra 0 e 1024.
- **Chiusura Connessione**: Per chiudere una connessione lato client digitare: /exit.
- **Finestre**: Per una maggiore leggibilità dei messaggi è consigliato di usare lo schermo intero.

## Licenza
Multiple-Chat ha una licenza GNU General Public License v3.0.

## Contatti
Puoi contattarmi presso questo indirizzo email: ignazioandsperandeo@gmail.com