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
   * - | **Aether --> AetherHeaven**
     - | :greenBold:`4.4`
     - | Proposed; the grade is the maintainer's to set. Aether is ONE mesh (``head``, ``body``) and AetherHeaven is
       | THREE components (an unnamed main mesh, ``Bang``, ``Eye``), so the mod is split per component and each
       | part's blend weights are remapped through its own row.
       |
       | - The skin draws its OWN face, and a face texture cannot be moved from his face to the skin's: on the
       | skin, a mod that repaints his face shows the skin's face instead.
       |
       | - His back cape hangs half from the skin's back coat chain and half from its upper torso, so it keeps
       | his narrow centre slit; its hem can sit a little higher than on his own outfit.
       |
       | - A cool-coloured surface (a white or blue-grey mane, a lilac cape) keeps its cool shade; a warm one
       | shades as on his own outfit.
       |
       | - **Limitation:** a glowing eye painted into his body texture (a mask's eye, say) shows white on the
       | skin: the skin's eye shader does not colour a glow.
       |
       | - **Limitation:** white cloth shades a little warmer (peach-white) in shadow than on his own outfit, and
       | cream cloth a little cooler.
       |
       | - **Limitation:** a mod that gives an object no section of its own, drawing it only through its index
       | buffer's ``drawindexed = auto``, has that object missing on the skin.
   * - | **AetherHeaven --> Aether**
     - | :greenBold:`4.3`
     - | Proposed; the grade is the maintainer's to set. The inverse: the skin's main mesh, ``Bang`` and ``Eye``
       | merged onto his one mesh -- the bangs, the eyes and the hair of the main mesh onto his ``head``, the rest
       | onto his ``body``.
       |
       | - His own face is drawn, for the same reason as the other way round: a mod repainting the skin's face
       | keeps his face on him.
       |
       | - The coat's long emblem drape at the back rides his cape chain and is pulled in towards his middle, as
       | the skin's own bones carry it; the coat's outer hem panels flare a little wider than on the skin.
       |
       | - A mod that hides the skin's bangs shows no bangs on him either.
       |
       | - A mod that binds a slot's textures in the game's own register order is read by the textures' names, and
       | the missing ones are fetched from the game.
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
   * - | **Changli --> ChangliNymph**
     - | :greenBold:`4`
     - | Proposed; the grade is the maintainer's to set. Both characters are ONE mesh drawn as eight
       | components over a merged skeleton past 256 bones, so the fix writes Wuthering Waves' blend-remap
       | buffers itself.
       |
       | - Changli carries four bone influences a vertex and the skin's shaders read eight, so the remapped
       | blend is written eight wide.
       |
       | - Her skeleton was renumbered twice (1.1 -> 1.2 and 1.3 -> 1.4), and a mod is read in the
       | numbering its own geometry fits; a mod exported before WWMI's merged skeleton is lifted from each
       | component's own bone indices.
       |
       | - A mod made before her 2.6 update names older texture hashes; those are known by role, so the
       | mod's own art is carried rather than replaced by the game's.
       |
       | - Her coat, shoulder drapes and side sashes (and whatever a mod puts in their place) are drawn
       | through the skin's body, lit by its body shader. The skin's skirt, which draws them with her own
       | coat's shader in the character menu, cuts them away in the overworld.
       |
       | - Her blindfold is the skin's own (a separate mesh with other UVs), recoloured to her black with
       | its gold laurel kept: the fix downloads that recoloured texture. A mod that hides or watches her
       | blindfold does the same to the skin's.
       |
       | - **Limitation:** her chest flame tattoo is not drawn on the skin. Her own outfit cuts its shape
       | out of a glow texture and draws it as a small dark mark; the skin has no such shader.
       |
       | - **Limitation:** the flame feathers on her arm are drawn in their colours but do not glow. Her own
       | shader lights them from her detail map; the skin's body shader has no input that lights a part, and
       | the skin's own arm has no feathers.
       |
       | - **Limitation:** a mod that draws a component from mesh files of its own (one mod's coat and
       | tattoo, under ``Meshes/Split`` and ``Meshes/Tattoo``) does not carry those parts yet.
       |
       | - A mod whose sections are keyed only on the game's bone-data buffer, with no ``vb0`` section, is
       | not recognised as hers and is left as it is.
   * - | **ChangliNymph --> Changli**
     - | :greenBold:`4`
     - | Proposed; the grade is the maintainer's to set. The inverse of the pair above, and NOT its mirror
       | image.
       |
       | - The skin carries eight bone influences a vertex and Changli's shaders read four, so each vertex
       | keeps its four heaviest, their weights scaled to the same total (under one percent of the skin's
       | vertices use more than four).
       |
       | - Her body is drawn through Changli's legs slot, whose shader reads the same three textures; her
       | skirt and shawl through Changli's coat slot, the skirt's own shader.
       |
       | - **Limitation:** her sheer dress and shawl are see-through on her own outfit, and Changli has no
       | see-through shader, so they are drawn opaque, and a mod's transparency toggles on them change little.
       | Their texture's transparency channel is clamped to Changli's own range, or her shader would make
       | the dress's gold embroidery glow, and what her outfit cuts away is filled with the colour beside it.
       |
       | - **Limitation:** her fine gold body chains are cut out of thin strips on her own outfit; on Changli the
       | whole strip draws, so they come out as thin gold-brown cords rather than fine chains (darker on the back).
       |
       | - Her side panels are modelled facing sideways, and her own idle swings the right one round to face the
       | front. Changli's idle does not, so seen from straight in front that panel is a thin line on Changli; it
       | hangs broad beside her thigh from every side.
       |
       | - Her blindfold is Changli's own (a separate mesh with other UVs), recoloured to the skin's white lace:
       | the fix downloads that recoloured texture. It has no lace pattern or gold laurel, which Changli's
       | blindfold has no room for. A mod that hides or watches her blindfold does the same to Changli's.
   * - | **Charlotte --> CharlotteHurlock**
     - | :greenBold:`4.5`
     - | Charlotte is ONE mesh (``head`` and ``body``) and CharlotteHurlock is FOUR components (``Body`` of
       | five draw slots, ``Bangs``, ``Eyes``, ``Camera``), so the mod is split per component and each half's
       | blend weights are remapped through its own row.
       |
       | - Her fringe is weighted to her head, so no mod geometry reaches the skin's ``Bangs``: they are
       | hidden, and so is the skin's ``Camera``, an accessory rather than part of her.
       |
       | - A mod made before GI 6.x binds its textures in the old register order and renders flat on
       | Charlotte herself; it does the same on the skin.
   * - | **CharlotteHurlock --> Charlotte**
     - | :greenBold:`4.5`
     - | The inverse: the skin's ``Body``, ``Bangs`` and ``Eyes`` merged onto one mesh, laid end to end into one
       | set of buffers, every slot landing on Charlotte's ``body``.
       |
       | - The skin's ``Camera`` is not carried (Charlotte's own camera is a separate mesh the game draws
       | anyway), nor ``Body`` slot E, a lens drawn only in a special pass.
       |
       | - The skin draws its hair with a different shader, so on Charlotte the hair comes out slightly more
       | lavender than on the skin.
       |
       | - A mod that recolours the skin by texture hash alone (``this = ...``) has its textures carried onto
       | the slots that use them; a file that only watches the skin (a toggle or help menu) keeps its own
       | sections on Charlotte's hashes, so its keys still work.
       |
       | - A ``TexFx`` outline map a mod binds on one slot is kept on that slot (it can read bluer on Charlotte
       | than on the skin); it no longer spills onto the slots drawn after it.
   * - | **CherryHuTao --> HuTao**
     - | :greenBold:`4.6`
     - | - Front of HuTao's dress will clip to her legs when walking.
       |
       | No easy way to fix this since all the closest vertex groups on HuTao that could be mapped from CherryHuTao's front dress
       | result in the dress clipping her legs when she walks. (unless we start manipulating vertices of the models...)
       |
       | - We replace pink, yellow, green, blue regions with opacity (alpha) within 65-75 with an opaque green colour of rgba(0, 128, 0, 255) 
       | to fix HuTao's stockings. There may be a possibility that we replace more than necessary.
   * - | **Chisa --> ChisaParfait**
     - | :greenBold:`4.5`
     - | Proposed beside Sanhua --> SanhuaExorcist, the other pair on this template; the grade is the
       | maintainer's to set. Both characters are ONE mesh drawn as several components over a merged
       | skeleton, so the remap is per draw slot rather than per object, and a slot binds one set of
       | textures at a time.
       |
       | - ChisaParfait's right hip slot has nothing remapped onto it, so it keeps drawing the skin's
       | own geometry and textures.
       |
       | - Chisa's material mask is repacked into ChisaParfait's layout rather than bound as it is,
       | and which material a region becomes is decided by how flesh-coloured her diffuse is under it.
       | A mod that paints an unusual material on a region cannot be followed. The mask's green channel
       | (how shiny a surface is) is kept from the mod.
       |
       | - ChisaParfait's clothing shader reads a map that Chisa's has no input for. It is bound to a
       | flat neutral, so a surface that would vary across that map is uniform instead.
       |
       | - Chisa's hair ribbon is painted by a hair shader, and ChisaParfait has nowhere to draw it but
       | a cloth one, which renders the same texture darker. A colour grade over the ribbon's own UV
       | island puts most of that back, but the cloth shader adds an ambient floor of sRGB 44-52 per
       | channel and the ribbon renders AT it, so the last ~11 of green and ~15 of blue are not
       | reachable by any texture edit: brightness matches the base exactly, saturation is 0.585
       | against 0.659.
       |
       | - Her fox mask, hairpins and one skirt chain hang off bones ChisaParfait does not have. They
       | are anchored rigidly to a single bone each, so they keep their shape and their place but do
       | not follow the motion the source gave them.
       |
       | - A texture role the mod ships no file for is bound to Chisa's own game texture, downloaded ---
       | the mod's texture coordinates are hers, so only her textures agree with them.
       |
       | - Shape keys ARE retargeted for this pair, unlike Sanhua's.
       |
       | - The vertex group table was proposed from the geometry by ``Tools/VGRemapFinder`` rather than
       | made by hand.
       |
       | - A mod that binds a texture behind its own toggle is followed, one variant per branch. The
       | exception is a single variant under a real condition with no ``else``, which is bound
       | unconditionally.
   * - | **ChisaParfait --> Chisa**
     - | :greenBold:`4`
     - | The inverse of the pair above, and NOT its mirror image. Chisa's merged skeleton is 420 slots
       | where a blend index is 8 bits, so the fix writes Wuthering Waves' blend-remap buffers itself
       | and declares a skeleton at 512 bones rather than the 256 a mod of the skin declares.
       |
       | - Chisa has no counterpart for ChisaParfait's frilled panel or her right-hip prop. Both are
       | routed onto a Chisa slot that draws with the same shader family, so they are drawn and lit
       | correctly, and both ride the body's own bones rather than bones of their own --- all of the
       | prop's weight and 81% of the panel's. So they follow the body rather than moving the way the
       | skin's own garment does, and neither can be pinned to a single bone the way a prop with its
       | own bones would be, because pinning them would pin the body with them.
       |
       | - The vertex group table was proposed from the geometry by ``Tools/VGRemapFinder`` and then
       | reviewed by hand against the forward direction's, which is confirmed in game: the two agree
       | on 70% of the table outright, and every place they differ is within one bone's width.
       |
       | - ChisaParfait's clothing shader binds a detail map that Chisa's has no input for. It is
       | dropped rather than translated.
       |
       | - Her sheen is a holographic foil whose colour Chisa's shader cannot read at all, so the
       | register is left to the game and a mod that recolours the foil is not followed. The one
       | channel Chisa could use is her own matcap already.
       |
       | - The hair's detail map is repacked into Chisa's channel layout. One of Chisa's channels has
       | no counterpart in the skin's art and is written flat.
       |
       | - Checked in game on three mods, including the character's own model as a mod. No fault seen
       | at rest or in motion, from any angle.
   * - | **Citlali --> CitlaliWhisperofStars**
     - | :greenBold:`4.5`
     - | Citlali is ONE mesh (``head`` and ``body``) and CitlaliWhisperofStars is THREE components
       | (``Body`` of four draw slots, ``Bangs``, ``Eyes``), each with its own buffers, so the mod is split
       | per component and each half's blend weights are remapped through its own row.
       |
       | - A mod that draws an object as several TOGGLED ranges has each range remapped through the split,
       | since the skin's buffers renumber every vertex. A range the split drops entirely cannot be drawn.
       |
       | - The skin's ``Bangs`` and ``Eyes`` have no textures of their own --- the game draws them with the
       | ``Body``'s --- so a mod that repaints only one of the three is followed for that one and given the
       | game's own textures for the others.
       |
       | - **Citlali's two long front braids cannot be posed exactly.** They are modelled inside her body
       | at rest and her own hair bones carry them out at runtime; CitlaliWhisperofStars has no hair bone
       | below the collarbone, so her shorter front lock stands in and the rest is made up by displacing
       | the braid. That displacement is a fixed offset rather than a bone, so it cannot follow a pose:
       |
       |   - the braids pass through her chest when she **bends forward**, and
       |   - they overlap the loose hair beside them within a few millimetres, which the mod itself does
       |     and Citlali hides with bones the skin does not have.
       |
       | The offset is sized to keep them off her body while they still read as hanging against it; a
       | larger one clears more and starts to look detached. The last few centimetres of each braid stay
       | behind her dress panel, which is where the mod puts them.
   * - | **CitlaliWhisperofStars --> Citlali**
     - | :greenBold:`4.5`
     - | The inverse: three components merged onto one mesh, laid end to end into one set of buffers with
       | each component's blend weights remapped through its own reverse row first.
       |
       | - Every source slot lands on Citlali's ``body``, including the ``Bangs``: the skin draws its fringe
       | on the body's shader pass, and her ``head`` receives nothing. A mod whose fringe needs the head's
       | own shading cannot be expressed.
       |
       | - Citlali reads a normal map where Yelan and Bennett do not, so the skin's normal maps are carried
       | rather than dropped --- and a mod written in the GAME's register order rather than the fix
       | libraries' is re-slotted by the name of each texture it binds. A mod that names a texture after a
       | role it does not hold is followed into the wrong slot.
       |
       | - Her dress outline is her own: the skin outlines its skirt with shaders Citlali has no equivalent
       | for, so those two slots are kept out of her outline pass. A mod that adds geometry needing an
       | outline there does not get one.
       |
       | - The vertex group rows are proposals from geometry matching, not a hand-made draft, so a pose may
       | deform slightly at a joint the matcher guessed wrong.
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
   * - | **Lumine --> LumineHeaven**
     - | :greenBold:`4.4`
     - | Lumine is ONE mesh (``head``, ``body``, ``dress``) and LumineHeaven is THREE components (an unnamed main
       | mesh, ``Bang``, ``Eye``), so the mod is split per component and each half's blend weights are remapped
       | through its own row. Her body and her dress both go through the skin's body.
       |
       | - The skin draws its OWN face, and a face texture cannot be moved from her face to the skin's: on the
       | skin, a mod that repaints her face (make-up, a different face) shows the skin's face instead.
       |
       | - Her head's diffuse alpha is moved onto the skin's legend, or her hair renders a glowing orange.
       |
       | - Cloth on her ``dress`` (coats, capes, long skirt tails) is given an inside layer, or its lining renders
       | bright blue or dark on the skin. Where the mod models its own lining, the layer stays behind it.
       |
       | - A part of the mod that glows blue keeps its colour, and its TexFx glow is moved onto the skin's normal-map layout.
       |
       | - Layered clothes lose the outline of their inner layers, or it shows through the outer one as small dark
       | red marks.
       |
       | - Her centre front skirt panel has no centre counterpart on the skin and follows its right front skirt.
       |
       | - A mod whose own outfit is broken by a stale 4.0 hash renders right on the skin.
   * - | **LumineHeaven --> Lumine**
     - | :greenBold:`4.5`
     - | The inverse: the skin's main mesh, ``Bang`` and ``Eye`` merged onto one mesh -- the bangs and the eyes
       | onto Lumine's ``head``, the rest of the main mesh onto her ``body``; her own ``dress`` is hidden.
       |
       | - The skin's head mesh is split in two by what each part is: its hair is drawn as Lumine's hair, and its
       | sleeves, neck scarf and bow as her clothes, so neither takes the other's shading in shade.
       |
       | - Lumine's own face is drawn, for the same reason as the other way round: a mod repainting the skin's
       | face keeps Lumine's face on her.
       |
       | - The skin's flared cuffs, scarf tail and back bow ride Lumine's arm, spine and hip bones, so they do
       | not flutter as they do on the skin.
       |
       | - A mod that binds a slot's textures in the game's own register order (an eye with only a diffuse, say)
       | is read by the textures' names, and the missing ones are fetched from the game.
   * - | **Lynae --> LynaePeppermint**
     - | :greenBold:`4`
     - | Proposed; the grade is the maintainer's to set. Both characters are ONE mesh drawn as eight
       | components over a merged skeleton past 256 bones, so the fix writes Wuthering Waves' blend-remap
       | buffers itself.
       |
       | - Lynae's 3.7 update renumbered her skeleton, and a mod's ``vb0`` hash does not say which
       | numbering it was made in, so each mod is read in the numbering its own geometry fits.
       |
       | - Her parts are drawn on the skin's slots with Lynae's own shaders and material constants, her own
       | lighting ramp and her holographic foil, so her materials (film, satin, sheer tights, metal) shade as
       | on her own outfit.
       |
       | - **Limitation:** Lynae's props component --- her headphones, pin and ID card, and whatever a mod puts
       | there (one mod's hat brim, overskirt and sleeves) --- is drawn on her own outfit by a translucency pass
       | that the skin's outfit never runs. On the skin these parts draw opaque: a see-through brim or veil
       | comes out solid, and a part the mod marks as film comes out dimmer instead (one mod's white belt
       | flap, which also reads red and glows), or browner and less see-through (one mod's sheer pink top).
       |
       | - A mod that hands its textures or a glow to RabbitFX keeps its look: her shaders carry RabbitFX's
       | own patch, so each part reads the mod's maps the way RabbitFX routes them on her own outfit.
       |
       | - **Limitation:** the bottle she holds is a separate game object, not part of her outfit. The skin's
       | card shows the skin's own bottle, and a mod that replaces Lynae's (one mod's spray can) is not carried.
       |
       | - **Limitation:** small colour differences remain against her own outfit: the lower half of her tie is
       | a little brighter on every mod that keeps it, a holographic coat panel can take another hue (one
       | mod's lilac panel comes out olive, another's aqua grey-green), a small metal accessory comes out a
       | little lighter (one mod's chest bell), and a sheer black bra seen from below comes out solid. Her
       | shaders are given the same textures and constants on both outfits; the difference comes from the
       | skin's frame, and was not traced further.
   * - | **LynaePeppermint --> Lynae**
     - | :greenBold:`4`
     - | Proposed; the grade is the maintainer's to set. The inverse of the pair above, and NOT its
       | mirror image.
       |
       | - Lynae's props slot renders through a glass / foil layer that ordinary cloth barely reaches,
       | so nothing is drawn there. The skin's coat and her shirt, jacket and shoes both go through
       | Lynae's jacket slot, the second in a generated copy ``.ini`` that shares the mod's skeleton.
       |
       | - The skin marks the cloth under a sheer garment differently from Lynae, so every body mask is
       | repacked into Lynae's legend. Her see-through shirt and coat are drawn with the skin's own
       | see-through passes and show the body through them, as on the skin.
       |
       | - The lower body's outline pass is handed the skin's diffuse where Lynae's own pass reads a flat
       | white, so the outline follows the skin's own mask and does not show through her layered thigh
       | overlay.
       |
       | - **Limitation:** opaque details on the skin's see-through shirt and coat, where they lie over her
       | body --- a strap, a bow, a pendant, a sheer bikini top --- come out paler, cooler or bluer on Lynae than on
       | the skin. The see-through pass writes exactly the colour and opacity it writes on the skin (measured
       | in frame dumps of both); the difference is in how Lynae's frame lights the see-through surface.
       |
       | - Checked in game on five mods, including the character's own model as a mod, from every angle
       | and in every toggle state.
   * - | **Neuvillette --> NeuvilletteMelusent**
     - | :greenBold:`4.5`
     - | Neuvillette is ONE mesh (``head``, ``body``, ``dress``) and NeuvilletteMelusent is FOUR components (an
       | unnamed main mesh of three draw slots, ``Coat``, ``Bang``, ``Eye``), so the mod is split per component
       | and each half's blend weights are remapped through its own row.
       |
       | - His ``dress`` (his cravat, lace and cuff ruffles) is drawn through the skin's body slot; on a white
       | cravat the skin's cloth shading leaves a faint cyan cast in the shadows.
       |
       | - A component the mod leaves empty (a summer outfit with no coat) is hidden, not drawn from the skin.
       |
       | - A translucent ``TexFx`` shirt can vanish on the skin.
       |
       | - A mod whose textures are laid out in the game's own register order under misleading file names
       | (a "light map" that is really a diffuse) can come out in the wrong colours.
   * - | **NeuvilletteMelusent --> Neuvillette**
     - | :greenBold:`4.5`
     - | The inverse: the skin's main mesh, ``Coat``, ``Bang`` and ``Eye`` merged onto one mesh, laid end to end
       | into one set of buffers -- the head's textures onto Neuvillette's ``head``, the body's onto his ``body``.
       |
       | - The skin wears its coat like a mantle, its sleeves hanging empty behind the arms. Neuvillette has
       | nothing like it, so they follow his upper arms: loose when he raises an arm, rather than hanging.
       |
       | - A mod that recolours the skin by texture hash in a separate ``.ini`` beside its mesh has that
       | recolour carried onto the remapped mesh; a mod that is only a recolour draws the whole skin in it.
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
   * - | **Sanhua --> SanhuaExorcist**
     - | :greenBold:`4.5`
     - | Both characters are ONE mesh drawn as several components over a merged skeleton, so the remap is
       | per draw slot rather than per object, and a slot binds one set of textures at a time.
       |
       | - Sanhua's bodice, skirt AND arm skin all land on SanhuaExorcist's single torso slot. They are
       | drawn separately, each with its own bindings, and the extra draws go into their own ``.ini`` files.
       |
       | - SanhuaExorcist's torso shader reads a material mask where Sanhua's arm skin has none, so one is
       | invented from her measured skin code. A mod that paints an unusual material on the arms cannot be
       | followed.
       |
       | - Shape keys are not retargeted, so a mod that ships its own is drawn with the skin's expressions
       | instead, and the per-vertex shape key offsets the game streams are zeroed rather than remapped.
       |
       | - SanhuaExorcist's hair bun and trousers have nothing remapped onto them, so that slot keeps
       | drawing the skin's own geometry and textures.
       |
       | - A texture role the mod ships no file for is bound to Sanhua's own game texture, downloaded ---
       | the mod's texture coordinates are hers, so only her textures agree with them.
   * - | **SanhuaExorcist --> Sanhua**
     - | :greenBold:`4.5`
     - | The inverse, with the same per-slot limits. Two of Sanhua's slots --- her bangs and her arm skin ---
       | have nothing remapped onto them and keep drawing her own geometry.
       |
       | - SanhuaExorcist's bangs are drawn through Sanhua's HAIR slot and hair shader rather than her bangs
       | slot: her bangs shader reads the hair's material block shifted, with a warm row that turns a
       | dark-painted fringe brown. Sanhua's see-through bangs pass is therefore not run for them.
       |
       | - SanhuaExorcist's hair bun and trousers go through Sanhua's skirt slot, whose shader reads a
       | material mask that side of the remap has none of, so her plain-cloth code is invented for it.
       |
       | - Shape keys are not retargeted here either.
       |
       | - The vertex group table for this direction was reviewed from the geometry rather than hand-made,
       | so a pose may deform slightly at a joint it reads wrong.
   * - | **Shenhe <--> ShenheFrostFlower**
     - | :greenBold:`4.9`
     - |
   * - | **Xiangling <--> XianglingCheer**
     - | :greenBold:`5.0`
     - |
   * - | **Xingqiu <--> XingqiuBamboo**
     - | :greenBold:`4.9`
     - |
   * - | **Yaoyao --> YaoyaoBamboo**
     - | :greenBold:`4.4`
     - | Yaoyao is ONE mesh (``head``, ``body``) and YaoyaoBamboo is THREE components (an unnamed main mesh,
       | ``Bang``, ``Eye``), so the mod is split per component and each half's blend weights are remapped
       | through its own row.
       |
       | - Her head's light map and diffuse alpha are moved onto the skin's legend, or her hair and bells
       | render pale -- a blonde mod grey-green.
       |
       | - Her hair's inner layers (the faces turned in towards her head, or covered by another layer) draw no
       | outline on the skin. A mod whose long hair is built of close two-sided sheets showed small dark shards
       | there otherwise; the outer faces keep their outline, so the silhouette is unchanged.
       |
       | - A tassel on her basket hangs a little lower on the skin.
       |
       | - A mod whose own outfit is broken by a stale 4.0 hash renders right on the skin.
   * - | **YaoyaoBamboo --> Yaoyao**
     - | :greenBold:`4.6`
     - | The inverse: the skin's main mesh, ``Bang`` and ``Eye`` merged onto one mesh -- the head's textures
       | onto Yaoyao's ``head``, the body's onto her ``body``.
       |
       | - Her body shader reads the diffuse alpha as a glow, so the skin's body diffuse is written with alpha
       | 0, or the outfit renders lit up white.
       |
       | - A mod written on GIMI's newer ``SetTextures`` API can look wrong on the skin itself with an older
       | GIMI while it renders right on Yaoyao.
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