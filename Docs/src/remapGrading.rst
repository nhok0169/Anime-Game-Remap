.. role:: raw-html(raw)
    :format: html

.. role:: redBold
.. role:: orangeYellowBold
.. role:: greenBold


Remap Grading
===============

In general, remapping mods is a hacky process. For certain mods, it might not be
possible to fully copy a mod to a skin without slight distortions.

The quality of each remap below is rated on a scale from :redBold:`0.0` :orangeYellowBold:`-` :greenBold:`5.0`, where :redBold:`0.0` means the remap contains
many distortions and does not look like the original mod, while :greenBold:`5.0` represents the remap
is a perfect replica of the original mod.


Grading
--------
.. list-table::
   :widths: 40 15 45
   :header-rows: 1

   * - Remap
     - Grading
     - Notes
   * - | **Amber <--> AmberCN**
     - | :greenBold:`5.0`
     - |
   * - | **Arlcecchino --> ArlecchinoBoss**
     - | :greenBold:`5.0`
     - |
   * - | **Barbara <--> BarbaraSummertime**
     - | :greenBold:`5.0`
     - |
   * - | **Fischl --> FischlHighness**
     - | :greenBold:`5.0`
     - |
   * - | **FischlHighness --> Fischl**
     - | :greenBold:`4.5`
     - | Metal parts on Fischl's head will lose their lustre since Fischl's head does not have any metal map texture file
       | whereas FischlHighness does have a metal map texture file
       |
       | There was a design choice of either sacrifice the shadow of Fischl's hair or a bit of the texture on her crown, and
       |  the later was the result.
   * - | **Ganyu --> GanyuTwilight**
     - | :orangeYellowBold:`3.0`
     - | GanyuTwilight's hair will glow really brightly.
       | Currently, for the `headDiffuse.dds` file, you could do the following:
       |
       | 1. lower brightness of the file by about 50-55%
       | 2. perform an 'inver alpha' (invert the transparency channel) on the files
       | 
       | However, the above change still leaves a very slight tint discolouration
       | for Ganyu's hair
       |
       | Will see if there is a way to automate the above steps for those who are lazy 
       | to do the above on their own.
   * - | **GanyuTwilight --> Ganyu**
     - | :greenBold:`4.6`
     - |
   * - | **Jean <--> JeanCN**,
       | **Jean --> JeanSea**,
       | **JeanCN --> JeanSea**
     - | :greenBold:`5.0`
     - |
   * - | **JeanSea --> Jean**,
       | **JeanSea --> JeanCN**
     - | :greenBold:`4.8`
     - |
   * - | **Keqing <--> **KeqingOpulent**
     - | :greenBold:`5.0`
     - |
   * - | **Mona <--> MonaCN**
     - | :greenBold:`5.0`
     - |
   * - | **Nilou --> NilouBreeze**
     - | :greenBold:`4.7`
     - | Outline on NilouBreeze will have its colour changed.
       |
       | Can probably implement some sort of outline fix for NilouBreeze's outline, 
       | but have not found the hash to change the outline yet.
   * - | **Ningguang --> NingguangOrchid**
     - | :orangeYellowBold:`3.0`
     - | Ninguang's hair will glow really brightly.
       | Currently, for the `headDiffuse.dds` file, you could do the following:
       |
       | 1. lower brightness of the file by about 50-55%
       | 2. perform an 'inver alpha' (invert the transparency channel) on the file
       | 
       | However, the above change still leaves a very slight tint discolouration
       | for Ningguang's hair
       |
       | Will see if there is a way to automate the above steps for those who are lazy
       | to do the above on their own.
   * - | **NingguangOrchid --> Ningguang**
     - | :greenBold:`4.8`
     - | 
   * - | **Raiden --> RaidenBoss**
     - | :greenBold:`5.0`
     - |
   * - | **Rosaria <--> RosariaCN**
     - | :greenBold:`5.0`
     - |
   * - | **Shenhe <--> ShenheFrostFlower**
     - | :greenBold:`4.9`
     - |
