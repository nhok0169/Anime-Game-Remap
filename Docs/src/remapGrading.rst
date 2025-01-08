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
   * - | **Ayaka --> AyakaSpringBloom**
     - | :greenBold:`4.8`
     - | Hair may have sligthly less shadow 
       |
       | Requires `ORFix`_ to fix up AyakaSpringBloom's reflection
   * - | **AyakaSpringBloom --> Ayaka**
     - | :greenBold:`4.9`
     - | 
   * - | **Barbara <--> BarbaraSummertime**
     - | :greenBold:`5.0`
     - |
   * - | **Diluc --> DilucFlamme**
     - | :greenBold:`4.7`
     - | Pick your poison: 
       | 
       | 1. A dress that clips on DilucFlamme's legs, but can wave
       |    OR
       | 2. A dress that is positioned correctly, but cannot wave
       |
       | (We chose option 2.)
   * - | **DilucFlamme --> Diluc**
     - | :greenBold:`4.8`
     - | Tried to make Diluc's body skin tone match the skin tone of his face
       | by replacing black regions with alpha of 128 to have an alpha of 177.
       | There may be a possibility we replace more than necessary.
   * - | **Fischl --> FischlHighness**
     - | :greenBold:`5.0`
     - |
   * - | **FischlHighness --> Fischl**
     - | :greenBold:`4.5`
     - | Metal parts on Fischl's head will lose their lustre since Fischl's head does not have any metal map texture file
       | whereas FischlHighness does have a metal map texture file
       |
       | There was a design choice to either sacrifice the shadow of Fischl's hair or a bit of the texture on her crown, and
       | the later was the result.
   * - | **Ganyu --> GanyuTwilight**
     - | :greenBold:`4.2`
     - | GanyuTwilight's hair may be slightly different coloured.
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
   * - | **Keqing <--> KeqingOpulent**
     - | :greenBold:`5.0`
     - |
   * - | **Kirara <--> KiraraBoots**
     - | :greenBold:`4.6`
     - | 
   * - | **Klee --> KleeBlossomingStarlight**
     - | :greenBold:`4.8`
     - | Tried to make KleeBlossomingStarlight's body skin tone match the skin tone of her face
       | by replacing black regions with alpha 128 or alpha 255 with a more opaque green colour.
       | There may be a possibility we replace more than necessary.
   * - | **KleeBlossomingStarlight --> Klee**
     - | :greenBold:`4.8`
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
   * - | **NilouBreeze --> Nilou**
     - | :greenBold:`4.5`
     - | Requires `ORFix`_ to fix up Nilou's reflection
   * - | **Ningguang --> NingguangOrchid**
     - | :greenBold:`4.2`
     - | NinguangOrchid's hair may be slightly different coloured.
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
   * - | **Xingqiu <--> XingqiuBamboo**
     - | :greenBold:`4.9`
     - |


.. _ORFix: https://github.com/leotorrez/LeoTools/blob/main/releases/ORFix.ini