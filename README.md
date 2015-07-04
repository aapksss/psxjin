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

What does that really mean? It means that it emulates the way that a PS1 works and tries to translate PS1 machine language to PC language. This is very hard to do and we try as hard as we can to achieve the best emulator we possibly can.

PSXjin is an overhaul of the PCSX-rr fork of the PCSX emulator. Compared to previous forks, PSXjin uses virtually no plugins (only an input plugin is needed), a simple to use GUI, and a new SPU core.

<h1>How the emulator works:</h1>

- Put a BIOS in BIOS directory (recommended: scph1000.bin for NTSC-J games, scph1001.bin for NTSC games, and scph1002.bin for PAL games) [optional]
- Open the emulator
- Select "Open CD"" and pick a valid file type

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
Email: linuzappz@pcsx.net

Shadow<br>
Email: shadow@pcsx.net

Pete Bernett<br>
Email: psswitch@online.de

NoComp<br>
Email: NoComp@mailcity.com

Nik3d<br>
Email: None found

Akumax<br>
Email: akumax@pcsx.net

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
