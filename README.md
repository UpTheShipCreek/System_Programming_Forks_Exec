------Compilation kai Klhsh------
$ make
$ ./sniffer

------------Cleanup--------------
$ make clean (ka8arizei ta objectives, ta .out, ta .txt, to pipe kai to executable sniffer)


-----Concessions logw xronou-----
To sniffer kanei monitor to directory sto opoio trexei, h main mou einai void.

Gia input file "example.txt", ftiaxnei ena "example.txt.out" sto idio directory, 
    to opoio periexei ola ta URLs pou vrhke, me thn seira pou ta vrhke alla den ta apari8mei

Ama kapoio URL xwristei sta 2 (dhladh pesei sto endiameso apo 2 buffers), o worker den mporei na to katalavei

--------Alles shmeiwseis---------
O logos pou  to programma einai grammeno se C++ kai oxi C einai giati 
    gia thn apo8hkeysh PIDs twn workers exw xrhsimopoihsei ta STLs ths C++ <vector> kai <queue>

H diaxeirhsh twn files ginetai me Low Level I/O read/write, alla gia thn diaxeirhsh twn buffers/strings 
    exw xrhsimopoihsei kai diafores synarthseis ths <string.h>

O manager ftiaxnei osa workers osa ta lines pou toy epistrefei o listener to opoio einai kapws random
Merikes fores o listener grafei polla files sto idio buffer kai
merikes fores o manager prolavenei kai epe3ergazetai to buffer prwtoy o listener prolavei na grapsei allo arxeio

Meta to clean, ama kanw make, 3anatre3w to programma kai tou 3anadwsw to idia arxeia, 
oi workers den diavazoun tipota (logw twn file pointers na oti deixnoun sto telos twn files?)
O tropos pou to parakamptw einai me to na kanw clean, meta na metaferw ta arxeia pou 8elw na kanei sniff o sniffer sto directory, 
na trexw ton sniffer kai meta na 3anametaferw, ta idia arxeia kanontas replace ta palia.
Me ayton ton tropo (ginontai reset oi pointers? kai) oi workers mporoun na ta diavasoun.


-------------Genika--------------
H main arxizei me to na ftia3ei ena pipe gia thn epikoinwnia me ton listener kai sthn synexeia kanei fork()
O listener kanei exec thn inotifywait me redirect to ka8e output apo to SDTOUT sto pipe

O manager diavazei to output, vlepei posa lines einai kai kanonizei an 8a ftia3ei nea workers h oxi
me vash twn ari8mwn twn workers pou koimountai, oi opoioi einai apo8hkeumenoi sto worker_q

Sthn synexeia dinei to buffer stous workers oi opoioi diavazoun ta lines pou den exoun hdh diavastei 
(mesw tou line_number pou einai shared memory)
O ka8e worker kanei extract to name tou file apo kapoio sygkekrimeno line tou buffer,
ftiaxnei to .out xrhsimopoiontas to name pou diavase kai to anoigei
anoigei to file me to antistoixo name,
to diavazei me buffer mege8ous 4096 bytes sta opoia psaxnei gia URLs.
Ama vrei kapoio URL to grafei sto .out
Otan teliwsei stamataei (SIGSTOP) kai perimenei ton manager na ton 3ypnisei (SIGCONT)

