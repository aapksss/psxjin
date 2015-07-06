<h1>PSXjin</h1>
<h2>An original PlayStation emulator</h2>

<h1>THIS IS A ROUGH DRAFT OF THE README</h1>

<b><i>
We exported the project, because if we didn't, this project might have disappeared forever.
So if you're a project admin, developer, founder, or contributor to PSXjin, check out this issue page so that I can give you write access: https://code.google.com/p/psxjin/issues/detail?id=53
</i></b>

<h1>Contents:</h1>
<b>
>
- General
- How it works
- Credits

</b>

<h1>General</h1>

PSXjin is an original PlayStation emulator.

<b><i>NOT</i></b> a PSX emulator. The PSX is an entirely different system. The PSX was only released in Japan, and used the PS2 as a backbone for most of it's features. It was the first PlayStation system to utilize the XrossMediaBar. It was kind of like a PS2 with a built-in hard drive and DVR. This emulator emulates the original PlayStation console.

What does that really mean? It means that it emulates the way that a PS1 works and tries to translate PS1 machine language to PC language. This is very hard to do and we try as hard as we can to achieve the best emulator we possibly can.

PSXjin is an overhaul of the PCSX-rr fork of the PCSX emulator. Compared to previous forks, PSXjin uses virtually no plugins at all, a simple to use GUI, and a new SPU core.

<h1>How the emulator works:</h1>

- Put a BIOS in the BIOS directory
- Open the emulator
- Select "Open CD"" and pick a valid file type

BIOS Guide:<br>
https://github.com/piorrro33/psxjin/wiki/Original-PlayStation-BIOS-Guide

CPU Options:

- Disable XA decoding:
Disables XA sound, if you don't want sound this might speed games up

- SIO IRQ always enabled:
Enable this only when controllers or memory cards don't work,
but usually breaks compatibility with some games when turned on.

- SPU IRQ always enabled:
May make the game work, but usually causes issues

- Black and white movies:
Speed up for slow machines

- Disable CD-DA:
Disable CD audio

- Enable console output:
Displays the PS1 text output

- Enable interpreter CPU:
Enables interpretive CPU emulation (recompiler by default) It may be more compatible, but it's slower

- PS1 System Type:
Choose between automatic region detection, or choose it yourself

<h1>Credits:</h1>
<i>
The PSXjin team:<br>
zeromus<br>
adelikat<br>
Darkkobold<br>

The PCSX-rr team:<br>
mz<br>

The PCSX team:<br>

Linuzappz<br>
Shadow<br>
Pete Bernett<br>
NoComp<br>
Nik3d<br>
Akumax<br>

The PSXjin team would like to thank:

Ancient: for beta testing PCSX and bothering me to correct it<br>
Roor: help on CD-ROM decoding routine<br>
Duddie<br>
Tratax<br>
Kazzuya: for the great work on PSEMU, and for the XA decoder<br>
Calb: for help coding<br>
Twin: for the bot on #pcsx on efnet, and for help coding<br>
Lewpy: for plugin work<br>
Psychojak: for beta testing PSXjin and donating a large amount of games to Shadow<br>
JNS: for adding PEC support to PSXjin<br>
Antiloop: for help coding<br>
Segu: he knows what for<br>
Null: he also knows<br>
Bobbi<br>
cdburnout: for putting PSXjin news on their site and for hosting us<br>
D[j]: for hosting us<br>
Now3d: for code help and information<br>
Ricardo Diaz: Linux version may never have been released without his help<br>
Nuno Felicio: beta testing, suggestions and Portuguese documentation<br>
Shunt: for help coding<br>
Taka: contributing fixes for PSXjin<br>
jang2k: contributing fixes for PSXjin<br>

Last but not least:<br>

James: for all the friendly support<br>
Dimitris: for being a good friend to me<br>
Joan: The woman that made me crazy the last couple months<br>
