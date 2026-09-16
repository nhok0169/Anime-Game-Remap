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
     - | :greenBold:`4.8`
     - | Tried to make AyakaSpringbloom's skin tone to match her face and make her clothes not follow the same shading as her face
       | by performing the following fix:
       |
       | - Change all opaque green colour in the lightmap to become transparent (clothing shading)
       | - Change all transparent greeen and black colour in the lightmap to become an opaque green colour of rgba(0, 128, 0, 255) (skin tone matching)
       |
       | There may be a possibility that we replace more than necessary.
   * - | **Barbara <--> BarbaraSummertime**
     - | :greenBold:`5.0`
     - |
   * - | **Bennett --> BennettAdventure**
     - | :greenBold:`4.5`
     - | Bennett is ONE mesh and BennettAdventure is THREE (``Body``, ``Bang``, ``Eye``), each with its own buffers
       | and its own vertex group numbering, so the mod's geometry is split rather than copied. Bennett's hair
       | stays with his head on BennettAdventure's ``Body``, and her own ``Bang`` is hidden so her bangs do not
       | sit on top of his hair.
       |
       | - A material band is a shading ramp, and the two skins do not agree on the legend --- and the legend
       | is per object: band 0 is hair on Bennett's head and cloth on his body. The bands are moved per pixel,
       | conditioned on the diffuse underneath, so a mod whose colours are unusual for the material can be misread.
       |
       | - Only the textures BennettAdventure's slots read are kept. A mod's metal map and shadow ramp are
       | left to the game, since his slots read something else from those registers.
       |
       | - A merged mod is fixed per variant. The light map band move is decided from the FIRST variant's diffuse,
       | so a merged mod whose variants repaint the diffuse differently may have some bands misplaced on the others.
       |
       | - The vertex group rows are proposals from geometry matching, not a hand-made draft, so a pose may
       | deform slightly at a joint the matcher guessed wrong.
   * - | **BennettAdventure --> Bennett**
     - | :greenBold:`4.5`
     - | The inverse: three components merged onto one mesh, laid end to end into one set of buffers with each
       | component's blend weights remapped through its own reverse row first.
       |
       | - BennettAdventure's ``Bang`` and ``Eye`` land on Bennett's ``head``, which is ONE ``.ini`` section. Where
       | they need different textures they are drawn separately, each with its own bindings --- but a mod that
       | needs a THIRD texture set for a single component cannot be expressed.
       |
       | - A component the mod does not have is downloaded from the game's own assets, along with the textures
       | that agree with the game's texture coordinates.
       |
       | - The vertex group rows are proposals from geometry matching, not a hand-made draft, so a pose may
       | deform slightly at a joint the matcher guessed wrong.
   * - | **CherryHuTao --> HuTao**
     - | :greenBold:`4.6`
     - | - Front of HuTao's dress will clip to her legs when walking.
       |
       | No easy way to fix this since all the closest vertex groups on HuTao that could be mapped from CherryHuTao's front dress
       | result in the dress clipping her legs when she walks. (unless we start manipulating vertices of the models...)
       |
       | - We replace pink, yellow, green, blue regions with opacity (alpha) within 65-75 with an opaque green colour of rgba(0, 128, 0, 255) 
       | to fix HuTao's stockings. There may be a possibility that we replace more than necessary.
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
   * - | **HuTao --> CherryHuTao**
     - | :greenBold:`4.9`
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
   * - | **Kaeya --> KaeyaSailwind**
     - | :greenBold:`4.9`
     - | 
   * - | **KaeyaSailwind --> Kaeya**
     - | :orangeYellowBold:`3.0`
     - | Kaeya is just weird...
       | 
       | Whether Kaeya's entire body will load is kind of undeterministic.
       |
       | Seems like Kaeya's model is hitting some unknown edge case in GIMI when GIMI handles his index buffers (.ib files).
       | Funny thing is that even original KaeyaSailwind mods without any remaps also exhibit this same behaviour.
       | The reason such a bug has not been reported is probably due to the very little amount of KaeyaSailwind mods that have been made.
       |
       | To increase the likelihood of Kaeya to properly show up, you want to try to trigger a scenario where GIMI will flush Kaeya's mod files.
       | So here are some actions you can try:
       | - Switching between the character menu screen and the overworld
       | - Switching between Kaeya and KaeyaSailwind
       | - Reloading the mod
   * - | **Keqing --> KeqingOpulent**
     - | :greenBold:`5.0`
     - |
   * - | **KeqingOpulent --> Keqing**
     - | :greenBold:`4.8`
     - | To decrease the amount of reflection in Keqing, we had to the following change:
       |
       | - Make all dark purple and dark yellow regions in the ``HeadLightMap.dds`` to be opaque with alpha value 255
       |
       | There may be a possibility that we replace more than necessary.
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
   * - | **Lisa <--> LisaStudent**
     - | :greenBold:`4.9`
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
   * - | **Xiangling <--> XianglingCheer**
     - | :greenBold:`5.0`
     - |
   * - | **Xingqiu <--> XingqiuBamboo**
     - | :greenBold:`4.9`
     - |
   * - | **Yelan --> YelanTranquil**
     - | :greenBold:`4.6`
     - | Yelan is ONE mesh and YelanTranquil is THREE (``Body``, ``Bang``, ``Eye``), each with its own buffers
       | and its own vertex group numbering, so the mod's geometry is split three ways rather than copied.
       |
       | - A material band is a shading ramp, and the two skins do not agree on the legend. The bands are moved
       | per pixel, conditioned on the diffuse underneath, because a mod that is itself a PORT carries some
       | THIRD character's legend and cannot be assumed to follow either one. A mod whose colours are unusual
       | for the material can be misread --- white fur and a white eye sclera are the same colour.
       |
       | - The two skins have different head shapes, so the split parts sit on the target's skull approximately.
   * - | **YelanTranquil --> Yelan**
     - | :greenBold:`4.5`
     - | The inverse: three components merged onto one mesh, laid end to end into one set of buffers with each
       | component's blend weights remapped through its own reverse row first.
       |
       | - YelanTranquil's ``Bang`` and ``Eye`` both land on Yelan's ``head``, which is ONE ``.ini`` section, and a
       | section binds one set of textures at a time. Where the two need different textures they are drawn
       | separately, each with its own bindings --- but a mod that needs a THIRD texture set for a single
       | component cannot be expressed at all.
       |
       | - A component the mod does not have is downloaded from the game's own assets, which carry the game's
       | texture coordinates. If the mod also repainted its atlas and moved an island, only the game's textures
       | agree with those coordinates, so the downloaded component is given them. A mod that repaints the atlas
       | for a slot it DOES own is followed instead --- so a mod that does both, on the same slot, cannot be
       | satisfied both ways.
       |
       | - The vertex group rows are proposals from geometry matching, not a hand-made draft, so a pose may
       | deform slightly at a joint the matcher guessed wrong.


.. _ORFix: https://github.com/leotorrez/LeoTools/blob/main/releases/ORFix.ini