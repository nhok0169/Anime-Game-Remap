.. role:: raw-html(raw)
    :format: html

API Examples
============

Below are a few simple and common examples of using the API.

.. note::
    For more detailed information about the API, see :doc:`api`

:raw-html:`<br />`
:raw-html:`<br />`

Fixing .Ini Files
-----------------

Below are different ways of fixing either:

* A single .ini file :raw-html:`<br />` **OR**
* The content contained in a single .ini file

:raw-html:`<br />`

Only Fix a .ini File Given the File Path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. note::
    This example only fixes the .ini file without removing any previous changes the fix may have made. If you want to
    first undo previous changes the fix may have done, see 
    :ref:`Remove a Fix from a .ini File Given the File Path <Remove a Fix from a .ini File Given the File Path>`


    To fix the .ini file by first removing any previous changes the fix may have made, see 
    :ref:`Fix a .ini File Given the File Path <Fix a .ini File Given the File Path>`


:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: CuteLittleRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("CuteLittleRaiden.ini", modTypes = AGR.ModTypes.getAll())
        iniFile.parse()
        iniFile.fix()

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: CuteLittleRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf


        ; --------------- Raiden Remap ---------------
        ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** RaidenBoss *****
        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        run = CommandListRaidenShogunRaidenBossRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRaidenBossRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
            else
                vb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRaidenBossRemapBlend
        endif

        [SubSubTextureOverrideRaidenBossRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
        endif


        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

        ; **********************

        ; --------------------------------------------

:raw-html:`<br />`

Only Fix .Ini file Given Only a String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The code below will add the lines that make up the fix to the end of the original content of the .ini file

.. note::
    This example only fixes the .ini file without removing any previous changes the fix may have made. If you want to
    first undo previous changes the fix may have done, see 
    :ref:`Remove a Fix from a .ini File Given Only a String Containing the Content of the File <Remove a Fix from a .ini File Given Only a String Containing the Content of the File>`


    To fix the .ini file by first removing any previous changes the fix may have made, see 
    :ref:`Fix a .ini File Given Only A String Containing the Content of the File <Fix a .ini File Given Only A String Containing the Content of the File>`

.. note::
    To only get the lines that make up the fix without adding back to the original .ini file, see 
    :ref:`Get Only the Fix to the .Ini file Given Only a String Containing the Content of the File <Get Only the Fix to the .Ini file Given Only a String Containing the Content of the File>`

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        shortWackyRaidenIniTxt = r"""
        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf
        """


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:
        :lineno-start: 71

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = shortWackyRaidenIniTxt, modTypes = AGR.ModTypes.getAll())
        iniFile.parse()
        fixedResult = iniFile.fix()

        print(fixedResult)

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf



        ; --------------- Raiden Remap ---------------
        ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** RaidenBoss *****
        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        run = CommandListRaidenShogunRaidenBossRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRaidenBossRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
            else
                vb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRaidenBossRemapBlend
        endif

        [SubSubTextureOverrideRaidenBossRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
        endif


        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

        ; **********************

        ; --------------------------------------------

:raw-html:`<br />`

Get Only the Fix to the .Ini file Given Only a String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The code below will only generate the necessary lines needed to fix the .ini file

.. note::
    To have the fixed lines added back to the original content of the file, see
    :ref:`Only Fix .Ini file Given Only a String Containing the Content of the File <Only Fix .Ini file Given Only a String Containing the Content of the File>`


.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        shortWackyRaidenIniTxt = r"""
        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf
        """

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:
        :lineno-start: 71

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = shortWackyRaidenIniTxt, modTypes = AGR.ModTypes.getAll())
        iniFile.parse()
        fixCode = iniFile.getFixStr()

        print(fixCode)


.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:

        ; --------------- Raiden Remap ---------------
        ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** RaidenBoss *****
        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        run = CommandListRaidenShogunRaidenBossRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRaidenBossRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
            else
                vb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRaidenBossRemapBlend
        endif

        [SubSubTextureOverrideRaidenBossRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
        endif


        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

        ; **********************

        ; --------------------------------------------


:raw-html:`<br />`

Remove a Fix from a .ini File Given the File Path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: PartiallyFixedRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

        ; --------------------------------------------------------------


        ; --------------- Raiden Boss Fix -----------------
        ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

        [TextureOverrideRaidenShogunRemapBlend]
        run = CommandListRaidenShogunRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
            vb1 = ResourceRaidenShogunRemapBlend.0
            else
            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRemapBlend
        endif

        [SubSubTextureOverrideRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
        endif


        [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


        ; -------------------------------------------------


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("PartiallyFixedRaiden.ini", modTypes = AGR.ModTypes.getAll())
        iniFile.removeFix(keepBackups = False)


.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: PartiallyFixedRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------



:raw-html:`<br />`

Remove a Fix from a .ini File Given Only a String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        showWackyRaidenIniTxtWithFix = r"""
        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

        ; --------------------------------------------------------------


        ; --------------- Raiden Boss Fix -----------------
        ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

        [TextureOverrideRaidenShogunRemapBlend]
        run = CommandListRaidenShogunRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
            vb1 = ResourceRaidenShogunRemapBlend.0
            else
            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRemapBlend
        endif

        [SubSubTextureOverrideRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
        endif


        [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


        ; -------------------------------------------------
        """


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:
        :lineno-start: 148

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix, modTypes = AGR.ModTypes.getAll())
        fixCode = iniFile.removeFix(keepBackups = False)

        print(fixCode)


.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------




:raw-html:`<br />`

Fix a .ini File Given the File Path
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example is the combined result of these 2 examples:

* :ref:`Only Fix a .ini File Given the File Path <Only Fix a .ini File Given the File Path>`
* :ref:`Remove a Fix from a .ini File Given the File Path <Remove a Fix from a .ini File Given the File Path>`

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: PartiallyFixedRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

        ; --------------------------------------------------------------


        ; --------------- Raiden Boss Fix -----------------
        ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

        [TextureOverrideRaidenShogunRemapBlend]
        run = CommandListRaidenShogunRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
            vb1 = ResourceRaidenShogunRemapBlend.0
            else
            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRemapBlend
        endif

        [SubSubTextureOverrideRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
        endif


        [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


        ; -------------------------------------------------


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("PartiallyFixedRaiden.ini", modTypes = AGR.ModTypes.getAll())
        iniFile.removeFix(keepBackups = False)
        iniFile.parse()
        iniFile.fix()

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: PartiallyFixedRaiden.ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------




        ; --------------- Raiden Remap ---------------
        ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** RaidenBoss *****
        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        run = CommandListRaidenShogunRaidenBossRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRaidenBossRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
            else
                vb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRaidenBossRemapBlend
        endif

        [SubSubTextureOverrideRaidenBossRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
        endif


        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

        ; **********************

        ; --------------------------------------------
        

:raw-html:`<br />`

Fix a .ini File Given Only A String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example is the combined result of these 2 examples:

* :ref:`Only Fix .Ini file Given Only a String Containing the Content of the File <Only Fix .Ini file Given Only a String Containing the Content of the File>`
* :ref:`Remove a Fix from a .ini File Given Only a String Containing the Content of the File <Remove a Fix from a .ini File Given Only a String Containing the Content of the File>`

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        showWackyRaidenIniTxtWithFix = r"""
        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf

        ; --------------------------------------------------------------


        ; --------------- Raiden Boss Fix -----------------
        ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

        [TextureOverrideRaidenShogunRemapBlend]
        run = CommandListRaidenShogunRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
            vb1 = ResourceRaidenShogunRemapBlend.0
            else
            vb1 = ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRemapBlend
        endif

        [SubSubTextureOverrideRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRemapBlend
        endif


        [GIMINeedsResourcesToAllStartWithResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = ..\AAA\BBBB\CCCCCC\DDDDDRemapRemapBlend.buf

        [ResourceEiBlendsHerRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRemapBlend.buf
        else
            run = RaidenPuppetCommandResourceRemapBlend
        endif

        [ResourceRaidenShogunRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRemapBlend.buf

        [RaidenPuppetCommandResourceRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRemapBlend.buf


        ; -------------------------------------------------
        """


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:
        :lineno-start: 148

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix, modTypes = AGR.ModTypes.getAll())
        iniFile.removeFix(keepBackups = False)
        iniFile.parse()
        fixResult = iniFile.fix()

        print(fixResult)

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:

        [Constants]
        global persist $swapvar = 0
        global persist $swapvarn = 0
        global persist $swapmain = 0
        global persist $swapoffice = 0
        global persist $swapglasses = 0

        [KeyVar]
        condition = $active == 1
        key = VK_DOWN
        type = cycle
        $swapvar = 0,1,2

        [KeyIntoTheHole]
        condition = $active == 1
        key = VK_RIGHT
        type = cycle
        $swapvarn = 0,1

        ; The top part is not really important, so I not going to finish
        ;   typing all the key swaps... 😋
        ;
        ; The bottom part is what the fix actually cares about

        [TextureOverrideRaidenShogunBlend]
        run = CommandListRaidenShogunBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunBlend.0
            else
                vb1 = ResourceEiBlendsHerBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverride
        endif

        [SubSubTextureOverride]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = GIMINeedsResourcesToAllStartWithResource
        endif

        [ResourceRaidenShogunBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\../../../../../../2-BunnyRaidenShogun\RaidenShogunBlend.buf

        [ResourceEiBlendsHerBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEi.buf
        else
            run = RaidenPuppetCommandResource
        endif

        [GIMINeedsResourcesToAllStartWithResource]
        type = Buffer
        stride = 32
        filename = ./../AAA/BBBB\CCCCCC\DDDDDRemapBlend.buf

        [RaidenPuppetCommandResource]
        type = Buffer
        stride = 32
        filename = ./Dont/Use\If/Statements\Or/SubCommands\In/Resource\Sections.buf

        ; ------ some lines originally generated from the fix ---------




        ; --------------- Raiden Remap ---------------
        ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** RaidenBoss *****
        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        run = CommandListRaidenShogunRaidenBossRemapBlend
        handling = skip
        draw = 21916,0

        [CommandListRaidenShogunRaidenBossRemapBlend]
        if $swapmain == 0
            if $swapvar == 0 && $swapvarn == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
            else
                vb1 = ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie
            endif
        else if $swapmain == 1
            run = SubSubTextureOverrideRaidenBossRemapBlend
        endif

        [SubSubTextureOverrideRaidenBossRemapBlend]
        if $swapoffice == 0 && $swapglasses == 0
            vb1 = ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend
        endif


        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = ../AAA/BBBB/CCCCCC/DDDDDRemapRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:/AnotherDrive/CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = Dont/Use/If/Statements/Or/SubCommands/In/Resource/SectionsRaidenBossRemapBlend.buf

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ../../../../../../../../../2-BunnyRaidenShogun/RaidenShogunRaidenBossRemapBlend.buf

        ; **********************

        ; --------------------------------------------


:raw-html:`<br />`

Fixing a .ini File Without Showing the Mod on the Original Character
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows remapping a mod onto a character without the mod staying on the original character. 
By default, the mod will show on both the original character and the remapped character.

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: ini 
        :caption: AmberCN.ini
        :linenos:
        
        [TextureOverrideAmberCNBlend]
        vb1 = ResourceAmberCNBlend
        handling = skip
        draw = 21916,0

        [TextureOverrideAmberCNBody]
        hash = b41d4d94
        match_first_index = 5670
        ib = ResourceAmberCNBodyIB
        ps-t0 = ResourceAmberCNBodyDiffuse
        ps-t1 = ResourceAmberCNBodyLightMap
        ps-t2 = ResourceAmberCNBodyMetalMap
        ps-t3 = ResourceAmberCNBodyShadowRamp

        [ResourceAmberCNBlend]
        type = Buffer
        stride = 32
        filename = AmberCNBlend.buf

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("AmberCN.ini", modTypes = AGR.ModTypes.getAll(), hideOrig = True)
        iniFile.parse()
        iniFile.fix()

        print(fixResult)

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: AmberCN.ini
        :linenos:

        ;RemapFixHideOrig -->[TextureOverrideAmberCNBlend]
        ;RemapFixHideOrig -->vb1 = ResourceAmberCNBlend
        ;RemapFixHideOrig -->handling = skip
        ;RemapFixHideOrig -->draw = 21916,0
        ;RemapFixHideOrig -->
        ;RemapFixHideOrig -->[TextureOverrideAmberCNBody]
        ;RemapFixHideOrig -->hash = b41d4d94
        ;RemapFixHideOrig -->match_first_index = 5670
        ;RemapFixHideOrig -->ib = ResourceAmberCNBodyIB
        ;RemapFixHideOrig -->ps-t0 = ResourceAmberCNBodyDiffuse
        ;RemapFixHideOrig -->ps-t1 = ResourceAmberCNBodyLightMap
        ;RemapFixHideOrig -->ps-t2 = ResourceAmberCNBodyMetalMap
        ;RemapFixHideOrig -->ps-t3 = ResourceAmberCNBodyShadowRamp
        ;RemapFixHideOrig -->
        [ResourceAmberCNBlend]
        type = Buffer
        stride = 32
        filename = AmberCNBlend.buf


        ; --------------- AmberCN Remap ---------------
        ; AmberCN remapped by NK#1321 and Albert Gold#2696. If you used it to remap your AmberCN mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** Amber *****
        [TextureOverrideAmberCNAmberRemapBlend]
        vb1 = ResourceAmberCNAmberRemapBlend
        handling = skip
        draw = 21916,0


        [TextureOverrideAmberCNBodyAmberRemapFix]
        hash = b03c7e30
        match_first_index = 5670
        ib = ResourceAmberCNBodyIB
        ps-t0 = ResourceAmberCNBodyDiffuse
        ps-t1 = ResourceAmberCNBodyLightMap
        ps-t2 = ResourceAmberCNBodyMetalMap
        ps-t3 = ResourceAmberCNBodyShadowRamp


        [ResourceAmberCNAmberRemapBlend]
        type = Buffer
        stride = 32
        filename = AmberCNAmberRemapBlend.buf

        ; *****************

        ; ---------------------------------------------


:raw-html:`<br />`

Fixing a .ini File to a Specific Version of the Game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a weird use case of wanting to fix the .ini file to an older version of the game (eg. when Fontaine first came out --> before the 'Great Hash Update')

.. note::
    The hashes and the indices are changed in the new .ini file to the older version of the game (the fix basically travelled in time!).

    To fix an entire mod for a specific version of the game, where the vertex group remaps of the Blend.buf files will also be affected by the specific game version
    go to :ref:`Fixing Entire Mods to a Specific Version of the Game`

.. dropdown:: Input
    :animate: fade-in-slide-down

    .. code-block:: ini 
        :caption: AmberCN.ini
        :linenos:
        
        [TextureOverrideAmberCNBlend]
        vb1 = ResourceAmberCNBlend
        handling = skip
        draw = 21916,0

        [TextureOverrideAmberCNBody]
        hash = b41d4d94
        match_first_index = 5670
        ib = ResourceAmberCNBodyIB
        ps-t0 = ResourceAmberCNBodyDiffuse
        ps-t1 = ResourceAmberCNBodyLightMap
        ps-t2 = ResourceAmberCNBodyMetalMap
        ps-t3 = ResourceAmberCNBodyShadowRamp

        [ResourceAmberCNBlend]
        type = Buffer
        stride = 32
        filename = AmberCNBlend.buf

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("AmberCN.ini", modTypes = AGR.ModTypes.getAll(), version = 4.0)
        iniFile.parse()
        iniFile.fix()

        print(fixResult)

.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :caption: AmberCN.ini
        :linenos:

        [TextureOverrideAmberCNBlend]
        vb1 = ResourceAmberCNBlend
        handling = skip
        draw = 21916,0

        [TextureOverrideAmberCNBody]
        hash = b41d4d94
        match_first_index = 5670
        ib = ResourceAmberCNBodyIB
        ps-t0 = ResourceAmberCNBodyDiffuse
        ps-t1 = ResourceAmberCNBodyLightMap
        ps-t2 = ResourceAmberCNBodyMetalMap
        ps-t3 = ResourceAmberCNBodyShadowRamp

        [ResourceAmberCNBlend]
        type = Buffer
        stride = 32
        filename = AmberCNBlend.buf


        ; --------------- AmberCN Remap ---------------
        ; AmberCN remapped by NK#1321 and Albert Gold#2696. If you used it to remap your AmberCN mods pls give credit for "Nhok0169" and "Albert Gold#2696"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        ; ***** Amber *****
        [TextureOverrideAmberCNAmberRemapBlend]
        vb1 = ResourceAmberCNAmberRemapBlend
        handling = skip
        draw = 21916,0


        [TextureOverrideAmberCNBodyAmberRemapFix]
        hash = 9976d124
        match_first_index = 5670
        ib = ResourceAmberCNBodyIB
        ps-t0 = ResourceAmberCNBodyDiffuse
        ps-t1 = ResourceAmberCNBodyLightMap
        ps-t2 = ResourceAmberCNBodyMetalMap
        ps-t3 = ResourceAmberCNBodyShadowRamp


        [ResourceAmberCNAmberRemapBlend]
        type = Buffer
        stride = 32
        filename = AmberCNAmberRemapBlend.buf

        ; *****************

        ; ---------------------------------------------

:raw-html:`<br />`
:raw-html:`<br />`

Fixing Blend.buf Files
----------------------

Below are different ways of fixing either:

* A single .*Blend.buf file :raw-html:`<br />` **OR**
* The content contained in a single .*Blend.buf file

:raw-html:`<br />`


Get a New and Fixed Blend.buf file Given the File path to an Existing Blend.buf File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example will make the fixed Blend.buf and put it in the same folder where the program is ran 

.. dropdown:: Input
    :animate: fade-in-slide-down

    assume we have this file structure and we are running from a file called ``example.py``

    .. code-block::

        RaidenShogun
        |
        +--> LittleEiBlend.buf
        |
        +--> Mod
              |
              +--> example.py


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        AGR.Mod.blendCorrection("../LittleEiBlend.buf", AGR.ModTypes.Raiden.value, "PuppetEiGotRemapped.buf")


.. dropdown:: Result
    :animate: fade-in-slide-down

    A new ``.buf`` file called ``PuppetEiGotRemapped.buf`` is created that includes the fix to ``LittleEiBlend.buf``

    .. code-block::

        RaidenShogun
        |
        +--> LittleEiBlend.buf
        |
        +--> Mod
              |
              +--> example.py
              |
              +--> PuppetEiGotRemapped.buf


:raw-html:`<br />`

Create the Bytes to the Fixed Blend.buf File Given the Bytes of the Existing Blend.buf File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example will make the fixed Blend.buf and put it in the same folder where the program is ran 

.. dropdown:: Input
    :animate: fade-in-slide-down

    assume we have this file structure and we are running from a file called ``example.py``

    .. code-block::

        RaidenShogun
        |
        +--> LittleEiBlend.buf
        |
        +--> Mod
              |
              +--> example.py

    :raw-html:`<br />`

    assume ``example.py`` first reads in the bytes from ``LittleEiBlend.buf``

    .. code-block:: python
        :caption: example.py
        :linenos:

        inputBytes = None
        with open("../LittleEiBlend.buf", "rb") as f:
            inputBytes = f.read()


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:
        :lineno-start: 4

        import AnimeGameRemap as AGR

        fixedBytes = AGR.Mod.blendCorrection(inputBytes, AGR.ModTypes.Raiden.value)
        print(fixedBytes)


.. dropdown:: Result
    :animate: fade-in-slide-down

    The bytes that fixes ``LittleEiBlend.buf``



:raw-html:`<br />`
:raw-html:`<br />`

Fixing Entire Mods
------------------

The below examples simulate executing the entire script, but through the API

:raw-html:`<br />`

Fixing Many Mods
~~~~~~~~~~~~~~~~

In this example, by running the program called `example.py`, the fix will start from the ``RaidenShogun/Mod`` folder and will: 

#. Undo previous changes created by the fix
#. Fix all the files related to mods

.. note::
    We set the ``verbose`` parmeter to ``False`` to not print out the usual logging text when you run the script.
    If you want to print out the logging text, set ``verbose`` to ``True``

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 31

            RaidenShogun
            |
            +--> AnotherSubTree
            |      |
            |      +--> someFolder
            |             |
            |             +--> disconnectedSubTree.ini
            |
            +--> Mod
            |     |
            |     +--> folder
            |     |      |
            |     |      +--> folderInFolder
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree2.buf
            |     |
            |     +--> folder2
            |     |     |
            |     |     +--> folderInFolder2
            |     |            |
            |     |            +--> AnotherFolder
            |     |                   |
            |     |                   +--> disconnectedSubTree2.ini
            |     |
            |     +--> pythonScript
            |     |     |
            |     |     +--> Run
            |     |           |
            |     |           +--> example.py
            |     |
            |     +--> ei.ini
            |     |
            |     +--> ei2.ini
            |     |
            |     +--> RaidenShogunBlend.buf
            |
            +--> ParentNodeBlend.buf

    :raw-html:`<br />`

    Assume below are the content for each .ini file

    .. dropdown:: ei.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0


            [TextureOverrideRaidenShogunBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceRaidenShogunBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            [ResourceRaidenShogunBlend.1]
            type = Buffer
            stride = 32
            filename = ../ParentNodeBlend.buf

    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf

    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:
        
            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService = AGR.BossFixService(path = "../../", verbose = False, keepBackups = False)
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Contains the fixed files for the mods.


    New File Structure:

    .. dropdown:: File Strucuture
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 35

            RaidenShogun
            |
            +--> AnotherSubTree
            |      |
            |      +--> someFolder
            |             |
            |             +--> disconnectedSubTree.ini
            |
            +--> Mod
            |     |
            |     +--> folder
            |     |      |
            |     |      +--> folderInFolder
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree2.buf
            |     |             |
            |     |             +--> RaidenBossRemapBlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> RaidenBossRemapBlendToDisconnectedSubTree2.buf
            |     |
            |     +--> folder2
            |     |     |
            |     |     +--> folderInFolder2
            |     |            |
            |     |            +--> AnotherFolder
            |     |                   |
            |     |                   +--> disconnectedSubTree2.ini
            |     |
            |     +--> pythonScript
            |     |     |
            |     |     +--> Run
            |     |           |
            |     |           +--> example.py
            |     |
            |     +--> ei.ini
            |     |
            |     +--> ei2.ini
            |     |
            |     +--> RaidenShogunBlend.buf
            |     |
            |     +--> RaidenShogunRaidenBossRemapBlend.buf
            |
            +--> ParentNodeBlend.buf
            |
            +--> ParentNodeRaidenBossRemapBlend.buf


    :raw-html:`<br />`

    Below contains the new content of the fixed.ini files:

    .. dropdown:: ei.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0


            [TextureOverrideRaidenShogunBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceRaidenShogunBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            [ResourceRaidenShogunBlend.1]
            type = Buffer
            stride = 32
            filename = ../ParentNodeBlend.buf


            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** RaidenBoss *****
            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunRaidenBossRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [ResourceRaidenShogunRaidenBossRemapBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunRaidenBossRemapBlend.buf

            [ResourceRaidenShogunRaidenBossRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../ParentNodeRaidenBossRemapBlend.buf

            ; **********************

            ; --------------------------------------------

    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf


            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** RaidenBoss *****
            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunRaidenBossRemapBlend.buf

            ; **********************

            ; --------------------------------------------

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf


            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** RaidenBoss *****
            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/RaidenBossRemapBlendToDisconnectedSubTree.buf

            ; **********************

            ; --------------------------------------------

    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:
        
            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf


            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Raiden mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** RaidenBoss *****
            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/RaidenBossRemapBlendToDisconnectedSubTree2.buf

            ; **********************

            ; --------------------------------------------


:raw-html:`<br />`

Undo the Fix from Many Mods
~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example, by running the program called `example.py`, the fix will start from the ``RaidenShogun/Mod`` folder and undo all previous changes done by the script

.. note::
    We set the ``verbose`` parmeter to ``False`` to not print out the usual logging text when you run the script.
    If you want to print out the logging text, set ``verbose`` to ``True``

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Strucuture
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 35

            RaidenShogun
            |
            +--> AnotherSubTree
            |      |
            |      +--> someFolder
            |             |
            |             +--> disconnectedSubTree.ini
            |
            +--> Mod
            |     |
            |     +--> folder
            |     |      |
            |     |      +--> folderInFolder
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree2.buf
            |     |             |
            |     |             +--> RemapBlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> RemapBlendToDisconnectedSubTree2.buf
            |     |
            |     +--> folder2
            |     |     |
            |     |     +--> folderInFolder2
            |     |            |
            |     |            +--> AnotherFolder
            |     |                   |
            |     |                   +--> disconnectedSubTree2.ini
            |     |
            |     +--> pythonScript
            |     |     |
            |     |     +--> Run
            |     |           |
            |     |           +--> example.py
            |     |
            |     +--> ei.ini
            |     |
            |     +--> ei2.ini
            |     |
            |     +--> RaidenShogunBlend.buf
            |     |
            |     +--> RaidenShogunRemapBlend.buf
            |
            +--> ParentNodeBlend.buf
            |
            +--> ParentNodeRemapBlend.buf


    :raw-html:`<br />`

    Assume below are the content of each .ini file

    .. dropdown:: ei.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0


            [TextureOverrideRaidenShogunBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceRaidenShogunBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            [ResourceRaidenShogunBlend.1]
            type = Buffer
            stride = 32
            filename = ../ParentNodeBlend.buf


            ; --------------- Raiden Boss Fix -----------------
            ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

            [TextureOverrideRaidenShogunRemapBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [ResourceRaidenShogunRemapBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunRemapBlend.buf

            [ResourceRaidenShogunRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\ParentNodeRemapBlend.buf


            ; -------------------------------------------------

    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf


            ; --------------- Raiden Boss Fix -----------------
            ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

            [TextureOverrideRaidenShogunRemapBlend]
            vb1 = ResourceRaidenShogunRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRemapBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunRemapBlend.buf


            ; -------------------------------------------------

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf


            ; --------------- Raiden Boss Fix -----------------
            ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

            [TextureOverrideRaidenShogunRemapBlend]
            vb1 = ResourceRaidenShogunRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\Mod\folder\folderInFolder\RemapBlendToDisconnectedSubTree.buf


            ; -------------------------------------------------

    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:
        
            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf


            ; --------------- Raiden Boss Fix -----------------
            ; Raiden boss fixed by NK#1321 if you used it for fix your raiden pls give credit for "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 and Albert Gold#2696 for support

            [TextureOverrideRaidenShogunRemapBlend]
            vb1 = ResourceRaidenShogunRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceRaidenShogunRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\..\folder\folderInFolder\RemapBlendToDisconnectedSubTree2.buf


            ; -------------------------------------------------


.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService =AGR.RaidenBossFixService(path = "../../", verbose = True, keepBackups = False, undoOnly = True)
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the previous changes made by the script removed

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 31

            RaidenShogun
            |
            +--> AnotherSubTree
            |      |
            |      +--> someFolder
            |             |
            |             +--> disconnectedSubTree.ini
            |
            +--> Mod
            |     |
            |     +--> folder
            |     |      |
            |     |      +--> folderInFolder
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree.buf
            |     |             |
            |     |             +--> BlendToDisconnectedSubTree2.buf
            |     |
            |     +--> folder2
            |     |     |
            |     |     +--> folderInFolder2
            |     |            |
            |     |            +--> AnotherFolder
            |     |                   |
            |     |                   +--> disconnectedSubTree2.ini
            |     |
            |     +--> pythonScript
            |     |     |
            |     |     +--> Run
            |     |           |
            |     |           +--> example.py
            |     |
            |     +--> ei.ini
            |     |
            |     +--> ei2.ini
            |     |
            |     +--> RaidenShogunBlend.buf
            |
            +--> ParentNodeBlend.buf

    :raw-html:`<br />`

    Below is the new content for each .ini file

    .. dropdown:: ei.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0


            [TextureOverrideRaidenShogunBlend]
            if $swapvar == 0
                vb1 = ResourceRaidenShogunBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceRaidenShogunBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceRaidenShogunBlend.0]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            [ResourceRaidenShogunBlend.1]
            type = Buffer
            stride = 32
            filename = ../ParentNodeBlend.buf

    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf

    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:
        
            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf

:raw-html:`<br />`

Override the Default Remap for a Character
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example below shows an alternative method to remap `Kirara --> KiraraBoots` . The prorgram by the name `example.py`
overwrites the default implementation of fixing Kirara mods with the new alternative method.

Reference: https://gamebanana.com/posts/12191289

:raw-html:`<br />`

.. caution::
    Modifying the :class:`FixRaidenBoss2.ModType` object from the :class:`FixRaidenBoss2.ModTypes` enum will modify how the software fixes a certain character, shown in
    the example below. If this side effect is not what you inteneded, it is recommended to create a new copy of the :class:`FixRaidenBoss2.ModType` object 
    through the :class:`FixRaidenBoss2.GIBuilder` class, then modify the new copy.

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 3

            Mods
            |
            +--> example.py
            |
            +--> KiraraAlt.ini
            |
            +--> Neko.dds
            |
            +--> KiraraBlend.buf

    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Kirara.ini
            :linenos:

            ; Kirara

            ; Constants -------------------------

            ; Overrides -------------------------

            [TextureOverrideKiraraPosition]
            hash = b57d7fe2
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraBlend]
            hash = 01d54938
            vb1 = ResourceKiraraBlend
            handling = skip
            draw = 41553,0 

            [TextureOverrideKiraraTexcoord]
            hash = 33b3d6e5
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaise]
            hash = 6fb396da

            [TextureOverrideKiraraIB]
            hash = f6e9af7d
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraHead]
            hash = f6e9af7d
            match_first_index = 0
            ib = ResourceKiraraHeadIB
            ps-t0 = ResourceKiraraHeadNormalMap
            ps-t1 = ResourceKiraraHeadDiffuse
            ps-t2 = ResourceKiraraHeadLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraBody]
            hash = f6e9af7d
            match_first_index = 37128
            ib = ResourceKiraraBodyIB
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDiffuse
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraDress]
            hash = f6e9af7d
            match_first_index = 75234
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t1 = ResourceKiraraDressDiffuse
            ps-t2 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraFaceHeadNormalMap]
            hash = 6eb20522
            ps-t0 = ResourceKiraraFaceHeadNormalMap


            ; CommandList -----------------------

            ; Resources -------------------------

            [ResourceKiraraPosition]
            type = Buffer
            stride = 40
            filename = KiraraPosition.buf

            [ResourceKiraraBlend]
            type = Buffer
            stride = 32
            filename = KiraraBlend.buf

            [ResourceKiraraTexcoord]
            type = Buffer
            stride = 20
            filename = KiraraTexcoord.buf

            [ResourceKiraraHeadIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraHead.ib

            [ResourceKiraraBodyIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraBody.ib

            [ResourceKiraraDressIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraDress.ib

            [ResourceKiraraHeadNormalMap]
            filename = KiraraHeadNormalMap.dds

            [ResourceKiraraHeadDiffuse]
            filename = KiraraHeadDiffuse.dds

            [ResourceKiraraHeadLightMap]
            filename = KiraraHeadLightMap.dds

            [ResourceKiraraBodyNormalMap]
            filename = KiraraBodyNormalMap.dds

            [ResourceKiraraBodyDiffuse]
            filename = Neko.dds

            [ResourceKiraraBodyLightMap]
            filename = KiraraBodyLightMap.dds

            [ResourceKiraraDressNormalMap]
            filename = KiraraDressNormalMap.dds

            [ResourceKiraraDressDiffuse]
            filename = KiraraDressDiffuse.dds

            [ResourceKiraraDressLightMap]
            filename = KiraraDressLightMap.dds

            [ResourceKiraraFaceHeadNormalMap]
            filename = KiraraFaceHeadNormalMap.dds

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        # ==== Override how Kirara is fixed ======

        kiraraModType = AGR.ModTypes.Kirara.value

        # Edit Kirara's body so that her body's skin tone matches with her face
        #
        # -- Notes --:
        # If you do not like how we edit her body, you can play around with her BodyDiffuse.dds or her BodyLightMap.dds
        #   in your favourite image editor (Paint.net, Photoshop, etc...) or you can tweak the code below
        kiraraModType.iniParseBuilder = AGR.IniParseBuilder(AGR.GIMIObjParser, args = [{"head", "body"}], kwargs = {"texEdits": {
            "body": {"ps-t1": {"DarkenDiffuse": AGR.TexEditor(filters = [AGR.TexMetadataFilter(edits = {"gamma": AGR.ColourConsts.SRGBGamma.value})])}}
        }})

        kiraraModType.iniFixBuilder = AGR.IniFixBuilder(AGR.GIMIObjMergeFixer, args = [{"head": ["head", "body"], "body": ["body"]}], 
                                                        kwargs = {
                                                            "preRegEditFilters": [
                                                                AGR.RegTexEdit({"DarkenDiffuse": ["ps-t1"]})
                                                            ],
                                                            "postRegEditFilters": [
                                                                AGR.RegNewVals({"body": {"ib": "null"}})
                                                            ]
                                                        })

        # ========================================

        # fix the mod
        remapService = AGR.RemapService(verbose = True, keepBackups = False)
        remapService.fix()

.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the previous changes made by the script removed

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 3

            Mods
            |
            +--> example.py
            |
            +--> KiraraAlt.ini
            |
            +--> KiraraAltRemapFix1.ini
            |
            +--> Neko.dds
            |
            +--> KiraraBodyDiffuseKiraraBootsRemapTex0.dds
            |
            +--> KiraraBlend.buf
            |
            +--> KiraraKiraraBootsRemapBlend.buf

    :raw-html:`<br />`

    Below is the new content for each .ini file

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Kirara.ini
            :linenos:

            ; Kirara

            ; Constants -------------------------

            ; Overrides -------------------------

            [TextureOverrideKiraraPosition]
            hash = b57d7fe2
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraBlend]
            hash = 01d54938
            vb1 = ResourceKiraraBlend
            handling = skip
            draw = 41553,0 

            [TextureOverrideKiraraTexcoord]
            hash = 33b3d6e5
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaise]
            hash = 6fb396da

            [TextureOverrideKiraraIB]
            hash = f6e9af7d
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraHead]
            hash = f6e9af7d
            match_first_index = 0
            ib = ResourceKiraraHeadIB
            ps-t0 = ResourceKiraraHeadNormalMap
            ps-t1 = ResourceKiraraHeadDiffuse
            ps-t2 = ResourceKiraraHeadLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraBody]
            hash = f6e9af7d
            match_first_index = 37128
            ib = ResourceKiraraBodyIB
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDiffuse
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraDress]
            hash = f6e9af7d
            match_first_index = 75234
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t1 = ResourceKiraraDressDiffuse
            ps-t2 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraFaceHeadNormalMap]
            hash = 6eb20522
            ps-t0 = ResourceKiraraFaceHeadNormalMap


            ; CommandList -----------------------

            ; Resources -------------------------

            [ResourceKiraraPosition]
            type = Buffer
            stride = 40
            filename = KiraraPosition.buf

            [ResourceKiraraBlend]
            type = Buffer
            stride = 32
            filename = KiraraBlend.buf

            [ResourceKiraraTexcoord]
            type = Buffer
            stride = 20
            filename = KiraraTexcoord.buf

            [ResourceKiraraHeadIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraHead.ib

            [ResourceKiraraBodyIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraBody.ib

            [ResourceKiraraDressIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraDress.ib

            [ResourceKiraraHeadNormalMap]
            filename = KiraraHeadNormalMap.dds

            [ResourceKiraraHeadDiffuse]
            filename = KiraraHeadDiffuse.dds

            [ResourceKiraraHeadLightMap]
            filename = KiraraHeadLightMap.dds

            [ResourceKiraraBodyNormalMap]
            filename = KiraraBodyNormalMap.dds

            [ResourceKiraraBodyDiffuse]
            filename = Neko.dds

            [ResourceKiraraBodyLightMap]
            filename = KiraraBodyLightMap.dds

            [ResourceKiraraDressNormalMap]
            filename = KiraraDressNormalMap.dds

            [ResourceKiraraDressDiffuse]
            filename = KiraraDressDiffuse.dds

            [ResourceKiraraDressLightMap]
            filename = KiraraDressLightMap.dds

            [ResourceKiraraFaceHeadNormalMap]
            filename = KiraraFaceHeadNormalMap.dds


            ; --------------- Kirara Remap ---------------
            ; Kirara remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Kirara mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KiraraBoots *****
            [TextureOverrideKiraraKiraraBootsRemapBlend]
            hash = 53a2502b
            vb1 = ResourceKiraraKiraraBootsRemapBlend
            handling = skip
            draw = 41553,0


            [TextureOverrideKiraraPositionKiraraBootsRemapFix]
            hash = f8013ba9
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraTexcoordKiraraBootsRemapFix]
            hash = 596e8fe0
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaiseKiraraBootsRemapFix]
            hash = 4955fc99

            [TextureOverrideKiraraIBKiraraBootsRemapFix]
            hash = 846979e2
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraDressKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 80295
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t1 = ResourceKiraraDressDiffuse
            ps-t2 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraFaceHeadNormalMapKiraraBootsRemapFix]
            hash = HashNotFound
            ps-t0 = ResourceKiraraFaceHeadNormalMap

            [TextureOverrideKiraraHeadKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 0
            ib = ResourceKiraraHeadIB
            ps-t0 = ResourceKiraraHeadNormalMap
            ps-t1 = ResourceKiraraHeadDiffuse
            ps-t2 = ResourceKiraraHeadLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraBodyKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 36804
            ib = null
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDarkenDiffuseKiraraBootsRemapTex0
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix


            [ResourceKiraraKiraraBootsRemapBlend]
            type = Buffer
            stride = 32
            filename = KiraraKiraraBootsRemapBlend.buf

            [ResourceKiraraBodyDarkenDiffuseKiraraBootsRemapTex0]
            filename = NekoKiraraBootsRemapTex0.dds

            ; ***********************

            ; --------------------------------------------


    .. dropdown:: KiraraAltRemapFix1.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Kirara.ini
            :linenos:

            ; Kirara

            ; Constants -------------------------

            ; Overrides -------------------------

            [TextureOverrideKiraraPosition]
            hash = b57d7fe2
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraBlend]
            hash = 01d54938
            vb1 = ResourceKiraraBlend
            handling = skip
            draw = 41553,0 

            [TextureOverrideKiraraTexcoord]
            hash = 33b3d6e5
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaise]
            hash = 6fb396da

            [TextureOverrideKiraraIB]
            hash = f6e9af7d
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraHead]
            hash = f6e9af7d
            match_first_index = 0
            ib = ResourceKiraraHeadIB
            ps-t0 = ResourceKiraraHeadNormalMap
            ps-t1 = ResourceKiraraHeadDiffuse
            ps-t2 = ResourceKiraraHeadLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraBody]
            hash = f6e9af7d
            match_first_index = 37128
            ib = ResourceKiraraBodyIB
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDiffuse
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraDress]
            hash = f6e9af7d
            match_first_index = 75234
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t1 = ResourceKiraraDressDiffuse
            ps-t2 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraFaceHeadNormalMap]
            hash = 6eb20522
            ps-t0 = ResourceKiraraFaceHeadNormalMap


            ; CommandList -----------------------

            ; Resources -------------------------

            [ResourceKiraraPosition]
            type = Buffer
            stride = 40
            filename = KiraraPosition.buf

            [ResourceKiraraBlend]
            type = Buffer
            stride = 32
            filename = KiraraBlend.buf

            [ResourceKiraraTexcoord]
            type = Buffer
            stride = 20
            filename = KiraraTexcoord.buf

            [ResourceKiraraHeadIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraHead.ib

            [ResourceKiraraBodyIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraBody.ib

            [ResourceKiraraDressIB]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KiraraDress.ib

            [ResourceKiraraHeadNormalMap]
            filename = KiraraHeadNormalMap.dds

            [ResourceKiraraHeadDiffuse]
            filename = KiraraHeadDiffuse.dds

            [ResourceKiraraHeadLightMap]
            filename = KiraraHeadLightMap.dds

            [ResourceKiraraBodyNormalMap]
            filename = KiraraBodyNormalMap.dds

            [ResourceKiraraBodyDiffuse]
            filename = Neko.dds

            [ResourceKiraraBodyLightMap]
            filename = KiraraBodyLightMap.dds

            [ResourceKiraraDressNormalMap]
            filename = KiraraDressNormalMap.dds

            [ResourceKiraraDressDiffuse]
            filename = KiraraDressDiffuse.dds

            [ResourceKiraraDressLightMap]
            filename = KiraraDressLightMap.dds

            [ResourceKiraraFaceHeadNormalMap]
            filename = KiraraFaceHeadNormalMap.dds


            ; --------------- Kirara Remap ---------------
            ; Kirara remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Kirara mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KiraraBoots *****
            [TextureOverrideKiraraKiraraBootsRemapBlend]
            hash = 53a2502b
            vb1 = ResourceKiraraKiraraBootsRemapBlend
            handling = skip
            draw = 41553,0


            [TextureOverrideKiraraPositionKiraraBootsRemapFix]
            hash = f8013ba9
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraTexcoordKiraraBootsRemapFix]
            hash = 596e8fe0
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaiseKiraraBootsRemapFix]
            hash = 4955fc99

            [TextureOverrideKiraraIBKiraraBootsRemapFix]
            hash = 846979e2
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraDressKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 80295
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t1 = ResourceKiraraDressDiffuse
            ps-t2 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraFaceHeadNormalMapKiraraBootsRemapFix]
            hash = HashNotFound
            ps-t0 = ResourceKiraraFaceHeadNormalMap

            [TextureOverrideKiraraHeadKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 0
            ib = ResourceKiraraBodyIB
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDarkenDiffuseKiraraBootsRemapTex1
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix


            [ResourceKiraraKiraraBootsRemapBlend]
            type = Buffer
            stride = 32
            filename = KiraraKiraraBootsRemapBlend.buf

            [ResourceKiraraBodyDarkenDiffuseKiraraBootsRemapTex1]
            filename = NekoKiraraBootsRemapTex0.dds

            ; ***********************

            ; --------------------------------------------

:raw-html:`<br />`

Remap Only a Few Selected Characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example, by running the program called `example.py`, the fix will only fix the mods for Keqing, Jean and Amber.
Mods for Shenhe and Raiden will not be fixed.

.. note::
    You can enter the nicknames/aliases of a mod in upper/lower case instead of just the regular name of the mod.
    Please refer to :ref:`Mod Types` for the available aliases for each mod.

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 10

            Mods
            |
            +--> Amber
            |    |
            |    +--> Amber.ini
            |    |
            |    +--> AmberBlend.buf
            |
            |
            +--> example.py
            |
            +--> Jean
            |    |
            |    +--> CuteJean
            |    |    |
            |    |    +--> CuteJean.ini
            |    |
            |    +--> SmolJean
            |    |    |
            |    |    +--> SmolJean.ini
            |    |    |
            |    |    +--> CuteJeanBlend.buf
            |    |
            |    +--> merged.ini
            |    |
            |    +--> SmolJeanBlend.buf
            |
            +--> Kequeen
            |    |
            |    +--> IniOrJoJ
            |    |     |
            |    |     +--> Cutie.ini
            |    |     |
            |    |     +--> Patootie.dds
            |    |     |
            |    |     +--> BestGurl.ini
            |    |     |
            |    |     +--> CatGirl.dds
            |    |     |
            |    |     +--> Cutesy.dds
            |    |     |
            |    |     +--> CutiePie.dds
            |    |
            |    +--> Buffs
            |          |
            |          +--> ISwearItsFor.buf
            |          |
            |          +--> SmallerHitboxes.buf
            |
            +--> Raiden
            |     |
            |     +--> KindOfGettingTired.buf
            |     |
            |     +--> WritingTheseTestCases.ini
            |
            +--> Yasu
                  |
                  +--> Endless9999GoldenTruth.ini
                  |
                  +--> DesDesDesDesDesDes.buf
                  |
                  +--> DieDaDes.buf
                  |
                  +--> SentenceToDes.buf
                  |
                  +--> DaGreatEqualizerIsTheDes.buf
    
    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: Amber.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Amber.ini
            :linenos:

            [TextureOverrideAmberBlend]
            vb1 = ResourceAmberBlend
            handling = skip
            draw = 21916,0

            [ResourceAmberBlend]
            type = Buffer
            stride = 32
            filename = AmberBlend.buf

    .. dropdown:: CuteJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: CuteJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            vb1 = ResourceJeanBlend
            handling = skip
            draw = 21916,0

            [ResourceJeanBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJean/CuteJeanBlend.buf

    .. dropdown:: SmolJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: SmolJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            vb1 = ResourceJeanBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanBody]
            hash = 115737ff
            match_first_index = 7779
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap

            [ResourceJeanBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJeanBlend.buf

    .. dropdown:: merged.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: merged.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideJeanBody]
            if $swapvar == 0
                hash = 115737ff
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            else if $swapvar == 1
                hash = 115737ff
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            endif

            [TextureOverrideJeanBlend]
            if $swapvar == 0
                vb1 = ResourceJeanBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceJeanBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanBlend.buf

            [ResourceJeanBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanBlend.buf

    .. dropdown:: Cutie.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Cutie.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [TextureOverrideKeqingHead]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingHead

            [CommandListKeqingHead]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingHeadDiffuse.0
                ps-t1 = ResourceKeqingHeadLightMap.0
                ps-t2 = ResourceKeqingHeadMetalMap.0
                ps-t3 = ResourceKeqingHeadShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingHeadIB.3
                ps-t0 = ResourceKeqingHeadDiffuse.3
                ps-t1 = ResourceKeqingHeadLightMap.3
            endif

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds

    .. dropdown:: BestGurl.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: BestGurl.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds

    .. dropdown:: WritingTheseTestCases.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: WritingTheseTestCases.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = KindOfGettingTired.buf

    .. dropdown:: Endless9999GoldenTruth.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Endless9999GoldenTruth.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1,2,3
            $creditinfo = 0

            [TextureOverrideShenheBlend]
            if $swapvar == 0
                vb1 = BeatoPleaseSaveNewbieGamemasterBattlerFromHisSmallBombsLogic
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = DlanorAKnoxDesDesDesDesDes
                handling = skip
                draw = 21916,0
            else if $swapvar == 2
                vb1 = YasuGirlOrBoyBetterLeaveQuestionInTheCatBox
                handling = skip
                draw = 21916,0
            else if $swapvar == 3
                vb1 = RosaDoubleWinchesterRiflesBadass
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideShenheDress]
            hash = 33a92492
            match_first_index = 48753
            run = CommandListShenheDress

            [CommandListShenheDress]
            if $swapvar == 0
                ib = ResourceShenheDressIB.0
                ps-t0 = ResourceShenheDressDiffuse.0
                ps-t1 = ResourceShenheDressLightMap.0
                ps-t2 = ResourceShenheDressMetalMap.0
                ps-t3 = ResourceShenheDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceShenheDressIB.1
                ps-t0 = ResourceShenheDressDiffuse.1
                ps-t1 = ResourceShenheDressLightMap.1
                ps-t2 = ResourceShenheDressMetalMap.1
                ps-t3 = ResourceShenheDressShadowRamp.1
            else if $swapvar == 2
                ib = ResourceShenheDressIB.2
                ps-t0 = ResourceShenheDressDiffuse.2
                ps-t1 = ResourceShenheDressLightMap.2
                ps-t2 = ResourceShenheDressMetalMap.2
                ps-t3 = ResourceShenheDressShadowRamp.2
            else if $swapvar == 3
                ib = ResourceShenheDressIB.3
                ps-t0 = ResourceShenheDressDiffuse.3
                ps-t1 = ResourceShenheDressLightMap.3
                ps-t2 = ResourceShenheDressMetalMap.3
                ps-t3 = ResourceShenheDressShadowRamp.3
            endif

            [BeatoPleaseSaveNewbieGamemasterBattlerFromHisSmallBombsLogic]
            type = Buffer
            stride = 32
            filename = DieDaDes.buf

            [DlanorAKnoxDesDesDesDesDes]
            type = Buffer
            stride = 32
            filename = DesDesDesDesDesDes.buf

            [YasuGirlOrBoyBetterLeaveQuestionInTheCatBox]
            type = Buffer
            stride = 32
            filename = SentenceToDes.buf

            [RosaDoubleWinchesterRiflesBadass]
            type = Buffer
            stride = 32
            filename = DaGreatEqualizerIsTheDes.buf

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService = AGR.RaidenBossFixService(verbose = True, keepBackups = False, undoOnly = True, types = ["kequeen", "aMbEr", "ACTINGGRANDMASTER"])
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the fix only being applied to Amber, Keqing and Jean

    .. dropdown:: File Strucuture
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 12

            Mods
            |
            +--> Amber
            |    |
            |    +--> Amber.ini
            |    |
            |    +--> AmberBlend.buf
            |    |
            |    +--> AmberAmberCNRemapBlend.buf
            |
            |
            +--> example.py
            |
            +--> Jean
            |    |
            |    +--> CuteJean
            |    |    |
            |    |    +--> CuteJean.ini
            |    |
            |    +--> SmolJean
            |    |    |
            |    |    +--> SmolJean.ini
            |    |    |
            |    |    +--> CuteJeanBlend.buf
            |    |    |
            |    |    +--> CuteJeanJeanCNRemapBlend.buf
            |    |    |
            |    |    +--> CuteJeanJeanSeaRemapBlend.buf
            |    |
            |    +--> merged.ini
            |    |
            |    +--> SmolJeanBlend.buf
            |    |
            |    +--> SmolJeanJeanCNRemapBlend.buf
            |    |
            |    +--> SmolJeanJeanSeaRemapBlend.buf
            |
            +--> Kequeen
            |    |
            |    +--> IniOrJoJ
            |    |     |
            |    |     +--> Cutie.ini
            |    |     |
            |    |     +--> Patootie.dds
            |    |     |
            |    |     +--> BestGurl.ini
            |    |     |
            |    |     +--> CatGirl.dds
            |    |     |
            |    |     +--> CutieRemapFix1.ini
            |    |     |
            |    |     +--> BestGurlRemapFix1.ini
            |    |     |
            |    |     +--> Cutesy.dds
            |    |     |
            |    |     +--> CutesyKeqingOpulentRemapTex0.dds
            |    |     |
            |    |     +--> CutiePie.dds
            |    |     |
            |    |     +--> CutiePieKeqingOpulentRemapTex0.dds
            |    |
            |    +--> Buffs
            |          |
            |          +--> ISwearItsFor.buf
            |          |
            |          +--> SmallerHitboxes.buf
            |          |
            |          +--> ISwearItsForKeqingOpulentRemapBlend.buf
            |          |
            |          +--> SmallerHitboxesKeqingOpulentRemapBlend.buf
            |
            +--> Raiden
            |     |
            |     +--> KindOfGettingTired.buf
            |     |
            |     +--> WritingTheseTestCases.ini
            |
            +--> Yasu
                  |
                  +--> Endless9999GoldenTruth.ini
                  |
                  +--> DesDesDesDesDesDes.buf
                  |
                  +--> DieDaDes.buf
                  |
                  +--> SentenceToDes.buf
                  |
                  +--> DaGreatEqualizerIsTheDes.buf


    :raw-html:`<br />`

    Below is the new content of the .ini files (and newly generated .ini files)

    .. dropdown:: Amber.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Amber.ini
            :linenos:

            [TextureOverrideAmberBlend]
            vb1 = ResourceAmberBlend
            handling = skip
            draw = 21916,0

            [ResourceAmberBlend]
            type = Buffer
            stride = 32
            filename = AmberBlend.buf


            ; --------------- Amber Remap ---------------
            ; Amber remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Amber mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** AmberCN *****
            [TextureOverrideAmberAmberCNRemapBlend]
            vb1 = ResourceAmberAmberCNRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceAmberAmberCNRemapBlend]
            type = Buffer
            stride = 32
            filename = AmberAmberCNRemapBlend.buf

            ; *******************

            ; -------------------------------------------

    .. dropdown:: CuteJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: CuteJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            vb1 = ResourceJeanBlend
            handling = skip
            draw = 21916,0

            [ResourceJeanBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJean/CuteJeanBlend.buf


            ; --------------- Jean Remap ---------------
            ; Jean remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Jean mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanJeanCNRemapBlend]
            vb1 = ResourceJeanJeanCNRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceJeanJeanCNRemapBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJean/CuteJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            vb1 = ResourceJeanJeanSeaRemapBlend
            handling = skip
            draw = 21916,0


            [ResourceJeanJeanSeaRemapBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJean/CuteJeanJeanSeaRemapBlend.buf

            ; *******************

            ; ------------------------------------------

    .. dropdown:: SmolJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: SmolJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            vb1 = ResourceJeanBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanBody]
            hash = 115737ff
            match_first_index = 7779
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap

            [ResourceJeanBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJeanBlend.buf


            ; --------------- Jean Remap ---------------
            ; Jean remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Jean mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanJeanCNRemapBlend]
            vb1 = ResourceJeanJeanCNRemapBlend
            handling = skip
            draw = 21916,0


            [TextureOverrideJeanBodyJeanCNRemapFix]
            hash = aad861e0
            match_first_index = 7779
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap


            [ResourceJeanJeanCNRemapBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            vb1 = ResourceJeanJeanSeaRemapBlend
            handling = skip
            draw = 21916,0


            [TextureOverrideJeanBodyJeanSeaRemapFix]
            hash = 69c0c24e
            match_first_index = 7662
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap

            [TextureOverrideJeanDressJeanSeaRemapFix]
            hash = 69c0c24e
            match_first_index = 52542
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap


            [ResourceJeanJeanSeaRemapBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJeanJeanSeaRemapBlend.buf

            ; *******************

            ; ------------------------------------------

    .. dropdown:: merged.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: merged.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideJeanBody]
            if $swapvar == 0
                hash = 115737ff
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            else if $swapvar == 1
                hash = 115737ff
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            endif

            [TextureOverrideJeanBlend]
            if $swapvar == 0
                vb1 = ResourceJeanBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceJeanBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanBlend.buf

            [ResourceJeanBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanBlend.buf


            ; --------------- Jean Remap ---------------
            ; Jean remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Jean mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanJeanCNRemapBlend]
            if $swapvar == 0
                vb1 = ResourceJeanJeanCNRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanJeanCNRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideJeanBodyJeanCNRemapFix]
            if $swapvar == 0
                hash = aad861e0
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            else if $swapvar == 1
                hash = aad861e0
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            endif


            [ResourceJeanJeanCNRemapBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanJeanCNRemapBlend.buf

            [ResourceJeanJeanCNRemapBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            if $swapvar == 0
                vb1 = ResourceJeanJeanSeaRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanJeanSeaRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideJeanBodyJeanSeaRemapFix]
            if $swapvar == 0
                hash = 69c0c24e
                match_first_index = 7662
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            else if $swapvar == 1
                hash = 69c0c24e
                match_first_index = 7662
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            endif

            [TextureOverrideJeanDressJeanSeaRemapFix]
            if $swapvar == 0
                hash = 69c0c24e
                match_first_index = 52542
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            else if $swapvar == 1
                hash = 69c0c24e
                match_first_index = 52542
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
            endif


            [ResourceJeanJeanSeaRemapBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanJeanSeaRemapBlend.buf

            [ResourceJeanJeanSeaRemapBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanJeanSeaRemapBlend.buf

            ; *******************

            ; ------------------------------------------

    .. dropdown:: Cutie.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Cutie.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [TextureOverrideKeqingHead]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingHead

            [CommandListKeqingHead]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingHeadDiffuse.0
                ps-t1 = ResourceKeqingHeadLightMap.0
                ps-t2 = ResourceKeqingHeadMetalMap.0
                ps-t3 = ResourceKeqingHeadShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingHeadIB.3
                ps-t0 = ResourceKeqingHeadDiffuse.3
                ps-t1 = ResourceKeqingHeadLightMap.3
            endif

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1
                ps-t1 = ResourceKeqingDressLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0]
            filename = CatGirlKeqingOpulentRemapTex0.dds

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1]
            filename = PatootieKeqingOpulentRemapTex0.dds

            ; *************************

            ; --------------------------------------------

    .. dropdown:: CutieRemapFix1.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: CutieRemapFix1.ini
            :linenos:

            ; This is really bad!! Don't do this!
            ; ************************************
            ;
            ; jk, but joking aside...
            ;
            ; The goal is to display n mod objects from the mod to be remapped to the mod onto a single mod object of the remapped mod.
            ;   Therefore we will have n sets of resources all mapping onto a single index (and same hash).
            ;
            ; Ideally, we would want all the sections to be within a single .ini file. The naive approach would be to create n sets of sections
            ;   (not a single section, cuz you need to include the case of sections depending on other sections, which form a section caller/callee graph) 
            ;    where the sections names are all unique. However, this approach will trigger a warning on GIMI (or any GIMI like importer) of multiple sections
            ;   mapping to the same hash and only 1 of the mod objects will be displayed
            ;
            ; The next attempt would be to take advantage of GIMI's overlapping mod bug/feature from loading multiple mods of the same character
            ;   Apart from the original .ini file, there would be n-1 newly generated .ini files (total of n .ini files). Each .ini file would uniquely
            ;   display a single set of sections from the n sets of sections. The overlapping property from the bug/feature would allow for all the objects to be displayed.
            ;
            ; For now, we were lazy and just simply copied the original .ini file onto the generated .ini files, which results in the original mod to have overlapping copies.
            ;  But since the mod used in all the .ini files are exactly the same, the user would not see the overlap (they may have some performance issues depending on the size of n. But
            ;   usually remaps only merge 2 mod objects into a single mod object, which should not cause much of an issue)
            ;   We could optimize the amount of space taken up by the newly generated .ini files, by only putting the necessary sections, but that is for another day...

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [TextureOverrideKeqingHead]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingHead

            [CommandListKeqingHead]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingHeadDiffuse.0
                ps-t1 = ResourceKeqingHeadLightMap.0
                ps-t2 = ResourceKeqingHeadMetalMap.0
                ps-t3 = ResourceKeqingHeadShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingHeadIB.3
                ps-t0 = ResourceKeqingHeadDiffuse.3
                ps-t1 = ResourceKeqingHeadLightMap.3
            endif

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingHeadOpaqueHeadDiffuseKeqingOpulentRemapTex0
                ps-t1 = ResourceKeqingHeadLightMap.0
                ps-t2 = ResourceKeqingHeadMetalMap.0
                ps-t3 = ResourceKeqingHeadShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingHeadIB.3
                ps-t0 = ResourceKeqingHeadOpaqueHeadDiffuseKeqingOpulentRemapTex1
                ps-t1 = ResourceKeqingHeadLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingHeadOpaqueHeadDiffuseKeqingOpulentRemapTex0]
            filename = CutesyKeqingOpulentRemapTex0.dds

            [ResourceKeqingHeadOpaqueHeadDiffuseKeqingOpulentRemapTex1]
            filename = CutiePieKeqingOpulentRemapTex0.dds

            ; *************************

            ; --------------------------------------------

    .. dropdown:: BestGurl.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: BestGurl.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1
                ps-t1 = ResourceKeqingDressLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0]
            filename = CatGirlKeqingOpulentRemapTex0.dds

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1]
            filename = PatootieKeqingOpulentRemapTex0.dds

            ; *************************

            ; --------------------------------------------

    .. dropdown:: BestGurlRemapFix1.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: BestGurlRemapFix1.ini
            :linenos:

            ; This is really bad!! Don't do this!
            ; ************************************
            ;
            ; jk, but joking aside...
            ;
            ; The goal is to display n mod objects from the mod to be remapped to the mod onto a single mod object of the remapped mod.
            ;   Therefore we will have n sets of resources all mapping onto a single index (and same hash).
            ;
            ; Ideally, we would want all the sections to be within a single .ini file. The naive approach would be to create n sets of sections
            ;   (not a single section, cuz you need to include the case of sections depending on other sections, which form a section caller/callee graph) 
            ;    where the sections names are all unique. However, this approach will trigger a warning on GIMI (or any GIMI like importer) of multiple sections
            ;   mapping to the same hash and only 1 of the mod objects will be displayed
            ;
            ; The next attempt would be to take advantage of GIMI's overlapping mod bug/feature from loading multiple mods of the same character
            ;   Apart from the original .ini file, there would be n-1 newly generated .ini files (total of n .ini files). Each .ini file would uniquely
            ;   display a single set of sections from the n sets of sections. The overlapping property from the bug/feature would allow for all the objects to be displayed.
            ;
            ; For now, we were lazy and just simply copied the original .ini file onto the generated .ini files, which results in the original mod to have overlapping copies.
            ;  But since the mod used in all the .ini files are exactly the same, the user would not see the overlap (they may have some performance issues depending on the size of n. But
            ;   usually remaps only merge 2 mod objects into a single mod object, which should not cause much of an issue)
            ;   We could optimize the amount of space taken up by the newly generated .ini files, by only putting the necessary sections, but that is for another day...

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            ; *************************

            ; --------------------------------------------

    .. dropdown:: WritingTheseTestCases.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: WritingTheseTestCases.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = KindOfGettingTired.buf

    .. dropdown:: Endless9999GoldenTruth.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Endless9999GoldenTruth.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1,2,3
            $creditinfo = 0

            [TextureOverrideShenheBlend]
            if $swapvar == 0
                vb1 = BeatoPleaseSaveNewbieGamemasterBattlerFromHisSmallBombsLogic
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = DlanorAKnoxDesDesDesDesDes
                handling = skip
                draw = 21916,0
            else if $swapvar == 2
                vb1 = YasuGirlOrBoyBetterLeaveQuestionInTheCatBox
                handling = skip
                draw = 21916,0
            else if $swapvar == 3
                vb1 = RosaDoubleWinchesterRiflesBadass
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideShenheDress]
            hash = 33a92492
            match_first_index = 48753
            run = CommandListShenheDress

            [CommandListShenheDress]
            if $swapvar == 0
                ib = ResourceShenheDressIB.0
                ps-t0 = ResourceShenheDressDiffuse.0
                ps-t1 = ResourceShenheDressLightMap.0
                ps-t2 = ResourceShenheDressMetalMap.0
                ps-t3 = ResourceShenheDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceShenheDressIB.1
                ps-t0 = ResourceShenheDressDiffuse.1
                ps-t1 = ResourceShenheDressLightMap.1
                ps-t2 = ResourceShenheDressMetalMap.1
                ps-t3 = ResourceShenheDressShadowRamp.1
            else if $swapvar == 2
                ib = ResourceShenheDressIB.2
                ps-t0 = ResourceShenheDressDiffuse.2
                ps-t1 = ResourceShenheDressLightMap.2
                ps-t2 = ResourceShenheDressMetalMap.2
                ps-t3 = ResourceShenheDressShadowRamp.2
            else if $swapvar == 3
                ib = ResourceShenheDressIB.3
                ps-t0 = ResourceShenheDressDiffuse.3
                ps-t1 = ResourceShenheDressLightMap.3
                ps-t2 = ResourceShenheDressMetalMap.3
                ps-t3 = ResourceShenheDressShadowRamp.3
            endif

            [BeatoPleaseSaveNewbieGamemasterBattlerFromHisSmallBombsLogic]
            type = Buffer
            stride = 32
            filename = DieDaDes.buf

            [DlanorAKnoxDesDesDesDesDes]
            type = Buffer
            stride = 32
            filename = DesDesDesDesDesDes.buf

            [YasuGirlOrBoyBetterLeaveQuestionInTheCatBox]
            type = Buffer
            stride = 32
            filename = SentenceToDes.buf

            [RosaDoubleWinchesterRiflesBadass]
            type = Buffer
            stride = 32
            filename = DaGreatEqualizerIsTheDes.buf


:raw-html:`<br />`

Fixing Entire Mods Without Showing Mods on the Original Character
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example below shows fixing entire mods where the mod only shows on the remapped character, and not the original character after running `example.py`.

.. note::
    To fix only a .ini file to a specific version of the game, go to :ref:`Fixing a .ini File Without Showing the Mod on the Original Character`

:raw-html:`<br />`


.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 15

            Mods
            |
            +--> Keqing
            |    |
            |    +--> changeVersionKeqing.ini
            |    |
            |    +--> CatGirl.dds
            |    |
            |    +--> Patootie.dds
            |    |
            |    +--> Cutesy.dds
            |    |
            |    +--> CutiePie.dds
            |    |
            |    +--> example.py
            |
            +--> Buffs
                 |
                 +--> SmallerHitboxes.buf

    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: changeVersionKeqing.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: changeVersionKeqing.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            [TextureOverrideKeqingBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingBody]
            hash = cbf1894b
            match_first_index = 10824
            run = CommandListKeqingBody

            [CommandListKeqingBody]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingDress]
            hash = cbf1894b
            match_first_index = 48216
            run = CommandListKeqingDress

            [CommandListKeqingDress]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuse.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuse.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService = AGR.RaidenBossFixService(verbose = True, keepBackups = False, undoOnly = True, hideOrig = True)
        fixService.fix()

.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the mod only applied only to KeqingOpulent, and not on Keqing.

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 25

            Mods
            |
            +--> Keqing
            |    |
            |    +--> changeVersionKeqing.ini
            |    |
            |    +--> changeVersionKeqingRemapFix1.ini
            |    |
            |    +--> CatGirl.dds
            |    |
            |    +--> CatGirlKeqingOpulentRemapTex0.dds
            |    |
            |    +--> Patootie.dds
            |    |
            |    +--> PatootieKeqingOpulentRemapTex0.dds
            |    |
            |    +--> Cutesy.dds
            |    |
            |    +--> CutesyKeqingOpulentRemapTex0.dds
            |    |
            |    +--> CutiePie.dds
            |    |
            |    +--> CutiePieKeqingOpulentRemapTex0.dds
            |    |
            |    +--> example.py
            |
            +--> Buffs
                 |
                 +--> SmallerHitboxes.buf
                 |
                 +--> SmallerHitboxesKeqingOpulentRemapBlend.buf

    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: changeVersionKeqing.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: changeVersionKeqing.ini
            :linenos:

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            ;RemapFixHideOrig -->[TextureOverrideKeqingBlend]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    vb1 = ResourceKeqingBlend.0
            ;RemapFixHideOrig -->    handling = skip
            ;RemapFixHideOrig -->    draw = 21916,0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    vb1 = ResourceKeqingBlend.1
            ;RemapFixHideOrig -->    handling = skip
            ;RemapFixHideOrig -->    draw = 21916,0
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[TextureOverrideKeqingBody]
            ;RemapFixHideOrig -->hash = cbf1894b
            ;RemapFixHideOrig -->match_first_index = 10824
            ;RemapFixHideOrig -->run = CommandListKeqingBody
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[CommandListKeqingBody]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    ib = ResourceKeqingBodyIB.0
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingBodyDiffuse.0
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingBodyLightMap.0
            ;RemapFixHideOrig -->    ps-t2 = ResourceKeqingBodyMetalMap.0
            ;RemapFixHideOrig -->    ps-t3 = ResourceKeqingBodyShadowRamp.0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    ib = ResourceKeqingBodyIB.3
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingBodyDiffuse.3
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingBodyLightMap.3
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[TextureOverrideKeqingDress]
            ;RemapFixHideOrig -->hash = cbf1894b
            ;RemapFixHideOrig -->match_first_index = 48216
            ;RemapFixHideOrig -->run = CommandListKeqingDress
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[CommandListKeqingDress]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    ib = ResourceKeqingDressIB.0
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingDressDiffuse.0
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingDressLightMap.0
            ;RemapFixHideOrig -->    ps-t2 = ResourceKeqingDressMetalMap.0
            ;RemapFixHideOrig -->    ps-t3 = ResourceKeqingDressShadowRamp.0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    ib = ResourceKeqingDressIB.3
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingDressDiffuse.3
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingDressLightMap.3
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1
                ps-t1 = ResourceKeqingDressLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex0]
            filename = CatGirlKeqingOpulentRemapTex0.dds

            [ResourceKeqingDressOpaqueDressDiffuseKeqingOpulentRemapTex1]
            filename = PatootieKeqingOpulentRemapTex0.dds

            ; *************************

            ; --------------------------------------------

    .. dropdown:: changeVersionKeqingRemapFix1.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: changeVersionKeqingRemapFix1.ini
            :linenos:

            ; This is really bad!! Don't do this!
            ; ************************************
            ;
            ; jk, but joking aside...
            ;
            ; The goal is to display n mod objects from the mod to be remapped to the mod onto a single mod object of the remapped mod.
            ;   Therefore we will have n sets of resources all mapping onto a single index (and same hash).
            ;
            ; Ideally, we would want all the sections to be within a single .ini file. The naive approach would be to create n sets of sections
            ;   (not a single section, cuz you need to include the case of sections depending on other sections, which form a section caller/callee graph) 
            ;    where the sections names are all unique. However, this approach will trigger a warning on GIMI (or any GIMI like importer) of multiple sections
            ;   mapping to the same hash and only 1 of the mod objects will be displayed
            ;
            ; The next attempt would be to take advantage of GIMI's overlapping mod bug/feature from loading multiple mods of the same character
            ;   Apart from the original .ini file, there would be n-1 newly generated .ini files (total of n .ini files). Each .ini file would uniquely
            ;   display a single set of sections from the n sets of sections. The overlapping property from the bug/feature would allow for all the objects to be displayed.
            ;
            ; For now, we were lazy and just simply copied the original .ini file onto the generated .ini files, which results in the original mod to have overlapping copies.
            ;  But since the mod used in all the .ini files are exactly the same, the user would not see the overlap (they may have some performance issues depending on the size of n. But
            ;   usually remaps only merge 2 mod objects into a single mod object, which should not cause much of an issue)
            ;   We could optimize the amount of space taken up by the newly generated .ini files, by only putting the necessary sections, but that is for another day...

            [Constants]
            global persist $swapvar = 0

            [KeySwap]
            condition = $active == 1
            key = VK_DOWN
            type = cycle
            $swapvar = 0,1
            $creditinfo = 0

            ;RemapFixHideOrig -->[TextureOverrideKeqingBlend]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    vb1 = ResourceKeqingBlend.0
            ;RemapFixHideOrig -->    handling = skip
            ;RemapFixHideOrig -->    draw = 21916,0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    vb1 = ResourceKeqingBlend.1
            ;RemapFixHideOrig -->    handling = skip
            ;RemapFixHideOrig -->    draw = 21916,0
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[TextureOverrideKeqingBody]
            ;RemapFixHideOrig -->hash = cbf1894b
            ;RemapFixHideOrig -->match_first_index = 10824
            ;RemapFixHideOrig -->run = CommandListKeqingBody
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[CommandListKeqingBody]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    ib = ResourceKeqingBodyIB.0
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingBodyDiffuse.0
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingBodyLightMap.0
            ;RemapFixHideOrig -->    ps-t2 = ResourceKeqingBodyMetalMap.0
            ;RemapFixHideOrig -->    ps-t3 = ResourceKeqingBodyShadowRamp.0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    ib = ResourceKeqingBodyIB.3
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingBodyDiffuse.3
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingBodyLightMap.3
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[TextureOverrideKeqingDress]
            ;RemapFixHideOrig -->hash = cbf1894b
            ;RemapFixHideOrig -->match_first_index = 48216
            ;RemapFixHideOrig -->run = CommandListKeqingDress
            ;RemapFixHideOrig -->
            ;RemapFixHideOrig -->[CommandListKeqingDress]
            ;RemapFixHideOrig -->if $swapvar == 0
            ;RemapFixHideOrig -->    ib = ResourceKeqingDressIB.0
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingDressDiffuse.0
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingDressLightMap.0
            ;RemapFixHideOrig -->    ps-t2 = ResourceKeqingDressMetalMap.0
            ;RemapFixHideOrig -->    ps-t3 = ResourceKeqingDressShadowRamp.0
            ;RemapFixHideOrig -->else if $swapvar == 1
            ;RemapFixHideOrig -->    ib = ResourceKeqingDressIB.3
            ;RemapFixHideOrig -->    ps-t0 = ResourceKeqingDressDiffuse.3
            ;RemapFixHideOrig -->    ps-t1 = ResourceKeqingDressLightMap.3
            ;RemapFixHideOrig -->endif
            ;RemapFixHideOrig -->
            [ResourceKeqingBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsFor.buf

            [ResourceKeqingBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxes.buf

            [ResourceKeqingDressDiffuse.0]
            filename = CatGirl.dds

            [ResourceKeqingDressDiffuse.3]
            filename = Patootie.dds

            [ResourceKeqingHeadDiffuse.0]
            filename = Cutesy.dds

            [ResourceKeqingHeadDiffuse.3]
            filename = CutiePie.dds


            ; --------------- Keqing Remap ---------------
            ; Keqing remapped by NK#1321 and Albert Gold#2696. If you used it to remap your Keqing mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** KeqingOpulent *****
            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif


            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingBodyKeqingOpulentRemapFix

            [CommandListKeqingBodyKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingBodyIB.0
                ps-t0 = ResourceKeqingBodyDiffuse.0
                ps-t1 = ResourceKeqingBodyLightMap.0
                ps-t2 = ResourceKeqingBodyMetalMap.0
                ps-t3 = ResourceKeqingBodyShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingBodyIB.3
                ps-t0 = ResourceKeqingBodyDiffuse.3
                ps-t1 = ResourceKeqingBodyLightMap.3
            endif


            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ../Buffs/ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ../Buffs/SmallerHitboxesKeqingOpulentRemapBlend.buf

            ; *************************

            ; --------------------------------------------


:raw-html:`<br />`

Fixing Entire Mods to a Specific Version of the Game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example below shows fixing entire mods to an older version of the game (before the 'Great Hash Update') after running `example.py`. If you do not specify any version, the fix will assume to fix to the latest game version available.

.. note::
    The hashes, indices and the vertex group remaps for the Blend.buf files are all fixed to the older version of the game. (The fix basically travelled in time!)

    To fix only a .ini file to a specific version of the game, go to :ref:`Fixing a .ini File to a Specific Version of the Game`

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 7

            AmberCN
            |
            +--> AmberCN.ini
            |
            +--> AmberCNBlend.buf
            |
            +--> example.py

    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: AmberCN.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: AmberCN.ini
            :linenos:

            [TextureOverrideAmberCNBlend]
            vb1 = ResourceAmberCNBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideAmberCNBody]
            hash = b41d4d94
            match_first_index = 5670
            ib = ResourceAmberCNBodyIB
            ps-t0 = ResourceAmberCNBodyDiffuse
            ps-t1 = ResourceAmberCNBodyLightMap
            ps-t2 = ResourceAmberCNBodyMetalMap
            ps-t3 = ResourceAmberCNBodyShadowRamp

            [ResourceAmberCNBlend]
            type = Buffer
            stride = 32
            filename = AmberCNBlend.buf

.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService = AGR.RaidenBossFixService(verbose = True, keepBackups = False, undoOnly = True, types = ["BaronBunnyCN"], version = 4.0)
        fixService.fix()

.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the fix applied for the game version 4.0

    .. dropdown:: File Strucuture
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 9

            AmberCN
            |
            +--> AmberCN.ini
            |
            +--> AmberCNBlend.buf
            |
            +--> AmberCNAmberRemapBlend.buf
            |
            +--> example.py

    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: AmberCN.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: AmberCN.ini
            :linenos:

            [TextureOverrideAmberCNBlend]
            vb1 = ResourceAmberCNBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideAmberCNBody]
            hash = b41d4d94
            match_first_index = 5670
            ib = ResourceAmberCNBodyIB
            ps-t0 = ResourceAmberCNBodyDiffuse
            ps-t1 = ResourceAmberCNBodyLightMap
            ps-t2 = ResourceAmberCNBodyMetalMap
            ps-t3 = ResourceAmberCNBodyShadowRamp

            [ResourceAmberCNBlend]
            type = Buffer
            stride = 32
            filename = AmberCNBlend.buf


            ; --------------- AmberCN Remap ---------------
            ; AmberCN remapped by NK#1321 and Albert Gold#2696. If you used it to remap your AmberCN mods pls give credit for "Nhok0169" and "Albert Gold#2696"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** Amber *****
            [TextureOverrideAmberCNAmberRemapBlend]
            vb1 = ResourceAmberCNAmberRemapBlend
            handling = skip
            draw = 21916,0


            [TextureOverrideAmberCNBodyAmberRemapFix]
            hash = 9976d124
            match_first_index = 5670
            ib = ResourceAmberCNBodyIB
            ps-t0 = ResourceAmberCNBodyDiffuse
            ps-t1 = ResourceAmberCNBodyLightMap
            ps-t2 = ResourceAmberCNBodyMetalMap
            ps-t3 = ResourceAmberCNBodyShadowRamp


            [ResourceAmberCNAmberRemapBlend]
            type = Buffer
            stride = 32
            filename = AmberCNAmberRemapBlend.buf

            ; *****************

            ; ---------------------------------------------