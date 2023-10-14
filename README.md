# lte-macro-micro_mobility


## Prerequisites (Debian/Ubuntu Linux)
```bash=
sudo apt-get install mercurial -y
sudo apt-get install qt5-default -y
sudo apt install qtchooser
```

## Building NetAnim

```bash=
make clean
qmake NetAnim.pro
make
```
`qmake NetAnim.pro` result:
```bash!
Info: creating stash file /home/bmwlab/Desktop/workspace/ns-allinone-3.40/netanim-3.109/.qmake.stash
```

## Starting NetAnim
**Path:**`/workspace/ns-allinone-3.40/netanim-3.109`
```bash=
./NetAnim
```

Build source code
> move your target file to /workspace/ns-allinone-3.40/ns-3.40/scratch/

**command Path:**`/workspace/ns-allinone-3.40/ns-3.40 `

```bash
./ns3 run scratch/<YOUR_TARGET_FILE.cc>
```
Run

**Path:**`/workspace/ns-allinone-3.40/ns-3.40 `

```bash
../netanim-3.109/NetAnim
```

**Reference:**
- :link: [[Video] installation of netanim in ns3 and testing on first file
](https://www.youtube.com/watch?v=HRAjKRrvgh4&list=PLRAV69dS1uWQEbcHnKbLldvzrjdOcOIdY&index=5&ab_channel=HiteshChoudhary)
- :link: https://www.nsnam.org/wiki/NetAnim_3.108
