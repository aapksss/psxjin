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

BIOS notes:

There seems to be a lot of confusion about the original PlayStation BIOS and it's revisions. Let me try to clear up any questions or misconceptions.

Let me start off by saying there are four main regions for the PS1:

- Japan
- Asia
- Europe
- North America

BIOS numbers follow a numbering system, as follows:

- Japan is any XX00 BIOS
- Asia is any XX03 BIOS
- Europe is any XX02 BIOS
- North America is any XX01 BIOS

By the way, all North American and Japanese games run at 60Hz (59.94Hz). European and Asian games run at 50Hz. So if you run a game with the "incorrect" BIOS, it will either give you a region error, or it will run at a different frame rate/refresh rate. This causes issues with many games that use frame rate/refresh rate for input, physics or AI calculations, or animation sync. Most games don't do this, however.

No matter what BIOS you use, in theory every officially licensed original PlayStation game for that BIOS' region should work.

The reason people recommend one BIOS over another is based on the features of the actual hardware the BIOS ran on. So, in the case of which North American BIOS to use, most people choose to use the SCPH-1001 BIOS because it had the most hardware features. They also choose this because there were no attempts at defeating piracy-enabling modchips in this BIOS (those didn't come until later). It also had parallel and serial ports which could be used for homebrew, dumps, etc. It can run nearly any homebrew, utilize the serial and parallel ports, etc.

There is also the fact that the SCPH-1001 is probably the most common PS1 console. Thus, by using that you are ensuring that it is a well-tested machine you are playing on.

The general consensus is that the most recommended BIOS is always the first launch console BIOS for the region that is the same as the game you are playing. So, if I want to play Metal Gear Solid, and I have the North American version, then you want to use the SCPH-1001 BIOS. If you want to play the Japanese version, then you would use the SCPH-1000 BIOS. And so on and so forth.

The BIOS files for development and special editions of the console have not been dumped, at least as far as we know, so that isn't really an issue that you should have to worry about.

TL;DR Use the SCPH-1000 BIOS for Japanese games, SCPH-1001 BIOS for North American games, SCPH-1002 BIOS for European games, and the SCPH-5003 BIOS for "Asian" games.

For homebrew, SDK demos, or Net Yaroze games, usually you can use any BIOS. I believe the Net Yaroze BIOS needs to be dumped still, and some games may not work, so beware of that.

We are aware of the pyramid head game not working, but that doesn't work on any emulator at the moment. I believe it actually modifies the BIOS in real-time, and it does so in a very buggy and inefficient way.

If you are wondering what the BIOS41A.bin file is that is floating around, I have no idea what it is. I haven't tested it. I will get to that, but until then I would recommend not using it, just because it will probably just not work.

If you have a system that hasn't been dumped, and you would like to donate it, feel free to email Turtle:

( vgturtle127 [at] gmail [dot] [com] )

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
