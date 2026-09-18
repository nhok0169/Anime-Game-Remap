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
    :ref:`Remove a Fix from a .ini File Given the File Path <apiExamples:Remove a Fix from a .ini File Given the File Path>`


    To fix the .ini file by first removing any previous changes the fix may have made, see 
    :ref:`Fix a .ini File Given the File Path <apiExamples:Fix a .ini File Given the File Path>`


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
        hash = 1a495487
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

        iniFile = AGR.IniFile("CuteLittleRaiden.ini")
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
        hash = 1a495487
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
        ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        hash = fe5c0180
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

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRaidenBossRemapBlend.buf

        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\..\AAA\BBBB\CCCCCC\DDDDDRemapRaidenBossRemapBlend.buf

        ; --------------------------------------------


:raw-html:`<br />`

Only Fix .Ini file Given Only a String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The code below will add the lines that make up the fix to the end of the original content of the .ini file

.. note::
    This example only fixes the .ini file without removing any previous changes the fix may have made. If you want to
    first undo previous changes the fix may have done, see 
    :ref:`Remove a Fix from a .ini File Given Only a String Containing the Content of the File <apiExamples:Remove a Fix from a .ini File Given Only a String Containing the Content of the File>`


    To fix the .ini file by first removing any previous changes the fix may have made, see 
    :ref:`Fix a .ini File Given Only A String Containing the Content of the File <apiExamples:Fix a .ini File Given Only A String Containing the Content of the File>`

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
        hash = 1a495487
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
        :lineno-start: 72

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = shortWackyRaidenIniTxt)
        iniFile.parse()
        fixedResult = iniFile.fix()

        for fixedTxt in fixedResult.values():
            print(fixedTxt)


.. dropdown:: Result
    :animate: fade-in-slide-down

    The printed text of the fixed .ini file

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
        hash = 1a495487
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
        ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        hash = fe5c0180
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

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRaidenBossRemapBlend.buf

        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\..\AAA\BBBB\CCCCCC\DDDDDRemapRaidenBossRemapBlend.buf

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
        hash = 1a495487
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

        iniFile = AGR.IniFile("PartiallyFixedRaiden.ini")
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
        hash = 1a495487
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
        hash = 1a495487
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
        :lineno-start: 149

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix)
        fixCode = iniFile.removeFix(keepBackups = False)

        print(fixCode)


.. dropdown:: Result
    :animate: fade-in-slide-down

    The printed text with the fix removed

    .. code-block:: ini
        :caption: IniWithFixRemoved.ini
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
        hash = 1a495487
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

* :ref:`Only Fix a .ini File Given the File Path <apiExamples:Only Fix a .ini File Given the File Path>`
* :ref:`Remove a Fix from a .ini File Given the File Path <apiExamples:Remove a Fix from a .ini File Given the File Path>`

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
        hash = 1a495487
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

        iniFile = AGR.IniFile("PartiallyFixedRaiden.ini")
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
        hash = 1a495487
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
        ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        hash = fe5c0180
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

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRaidenBossRemapBlend.buf

        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\..\AAA\BBBB\CCCCCC\DDDDDRemapRaidenBossRemapBlend.buf

        ; --------------------------------------------


:raw-html:`<br />`

Fix a .ini File Given Only A String Containing the Content of the File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example is the combined result of these 2 examples:

* :ref:`Only Fix .Ini file Given Only a String Containing the Content of the File <apiExamples:Only Fix .Ini file Given Only a String Containing the Content of the File>`
* :ref:`Remove a Fix from a .ini File Given Only a String Containing the Content of the File <apiExamples:Remove a Fix from a .ini File Given Only a String Containing the Content of the File>`

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
        hash = 1a495487
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
        :lineno-start: 149

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile(txt = showWackyRaidenIniTxtWithFix)
        iniFile.removeFix(keepBackups = False)
        iniFile.parse()
        fixedResult = iniFile.fix()

        for fixedTxt in fixedResult.values():
            print(fixedTxt)


.. dropdown:: Result
    :animate: fade-in-slide-down

    The printed text of the fixed .ini file

    .. code-block:: ini
        :caption: FixedIni.ini
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
        hash = 1a495487
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
        ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
        ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

        [TextureOverrideRaidenShogunRaidenBossRemapBlend]
        hash = fe5c0180
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

        [ResourceRaidenShogunRaidenBossRemapBlend.0]
        type = Buffer
        stride = 32
        filename = ..\..\..\..\..\..\..\..\..\2-BunnyRaidenShogun\RaidenShogunRaidenBossRemapBlend.buf

        [ResourceEiBlendsHerRaidenBossRemapBlenderInsteadOfHerSmoothie]
        type = Buffer
        stride = 32
        if $swapmain == 1
            filename = M:\AnotherDrive\CuteLittleEiRaidenBossRemapBlend.buf
        else
            run = ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend
        endif

        [ResourceRaidenPuppetCommandResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\Dont\Use\If\Statements\Or\SubCommands\In\Resource\SectionsRaidenBossRemapBlend.buf

        [ResourceGIMINeedsResourcesToAllStartWithResourceRaidenBossRemapBlend]
        type = Buffer
        stride = 32
        filename = .\..\AAA\BBBB\CCCCCC\DDDDDRemapRaidenBossRemapBlend.buf

        ; --------------------------------------------


:raw-html:`<br />`

Fixing a .ini File Without Showing the Mod on the Original Character
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows remapping a mod onto a character without the mod staying on the original character. 
By default, the mod will show on both the original character and the remapped character.

.. dropdown:: Input
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
        hash = 0bf8e621
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
        :linenos:

        import AnimeGameRemap as AGR

        iniFile = AGR.IniFile("changeVersionKeqing.ini")
        iniFile.parse()
        iniFile.fix(hideOrig = True)


.. dropdown:: Result
    :animate: fade-in-slide-down

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
            ;RemapFixHideOrig -->hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [ResourceKeqingHeadDiffuseRemapDL]
            filename = KeqingHeadDiffuseRemapDL.dds

            [ResourceKeqingHeadLightMapRemapDL]
            filename = KeqingHeadLightMapRemapDL.dds

            [ResourceKeqingHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KeqingHeadRemapDL.ib

            [ResourceKeqingFaceDiffuseRemapDL]
            filename = KeqingFaceDiffuseRemapDL.dds

            [ResourceKeqingPositionRemapDL]
            type = Buffer
            stride = 40
            filename = KeqingPositionRemapDL.buf

            [ResourceKeqingTexcoordRemapDL]
            type = Buffer
            stride = 20
            filename = KeqingTexcoordRemapDL.buf

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3
                ps-t1 = ResourceKeqingDressLightMap.3
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

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0]
            filename = KeqingOpulentHeadRemapTexKNs J93.dds

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3]
            filename = KeqingOpulentHeadRemapTexBkA J93.dds

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
            ;RemapFixHideOrig -->hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            ib = ResourceKeqingHeadIbRemapDL
            ps-t1 = ResourceKeqingHeadLightMapRemapDL
            hash = 7c6fc8c3
            match_first_index = 0
            ps-t0 = ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex]
            filename = KeqingOpulentHeadRemapTexCgN J93.dds

            ; --------------------------------------------



:raw-html:`<br />`

Fixing a .ini File to a Specific Version of the Game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example shows a weird use case of wanting to fix the .ini file to an older version of the game (eg. when Fontaine first came out --> before the 'Great Hash Update')

.. note::
    The hashes and the indices are changed in the new .ini file to the older version of the game (the fix basically travelled in time!).

    To fix an entire mod for a specific version of the game, where the vertex group remaps of the Blend.buf files will also be affected by the specific game version
    go to :ref:`Fixing Entire Mods to a Specific Version of the Game <apiExamples:Fixing Entire Mods to a Specific Version of the Game>`

.. dropdown:: Input
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
        hash = 0bf8e621
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
        :linenos:

        import AnimeGameRemap as AGR

        version = AGR.CppVersion.parse("4.0")

        # fromVersion: the version the mod was made for, toVersion: the version to fix the mod to
        iniFile = AGR.IniFile("changeVersionKeqing.ini", fromVersion = version, toVersion = version)
        iniFile.parse()
        iniFile.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

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
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [ResourceKeqingHeadDiffuseRemapDL]
            filename = KeqingHeadDiffuseRemapDL.dds

            [ResourceKeqingHeadLightMapRemapDL]
            filename = KeqingHeadLightMapRemapDL.dds

            [ResourceKeqingHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KeqingHeadRemapDL.ib

            [ResourceKeqingFaceDiffuseRemapDL]
            filename = KeqingFaceDiffuseRemapDL.dds

            [ResourceKeqingPositionRemapDL]
            type = Buffer
            stride = 40
            filename = KeqingPositionRemapDL.buf

            [ResourceKeqingTexcoordRemapDL]
            type = Buffer
            stride = 20
            filename = KeqingTexcoordRemapDL.buf

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 44bba21c
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDressDiffuseRemapTex.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDressDiffuseRemapTex.3
                ps-t1 = ResourceKeqingDressLightMap.3
            endif

            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 44bba21c
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

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t0 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDressDiffuseRemapTex.0]
            filename = KeqingOpulentDressRemapTexKNs OBu.dds

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDressDiffuseRemapTex.3]
            filename = KeqingOpulentDressRemapTexBkA OBu.dds

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

            [TextureOverrideKeqingBlend]
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            ib = ResourceKeqingHeadIbRemapDL
            ps-t1 = ResourceKeqingHeadLightMapRemapDL
            hash = 44bba21c
            match_first_index = 0
            ps-t0 = ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueHeadDiffuseRemapTex

            [TextureOverrideKeqingBodyKeqingOpulentRemapFix]
            hash = 44bba21c
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

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t0 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueHeadDiffuseRemapTex]
            filename = KeqingOpulentHeadRemapTexCgN FaT.dds

            ; --------------------------------------------



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

        vgRemap = AGR.ModTypes.Raiden.value.getVGRemap("RaidenBoss")
        AGR.BlendFile("../LittleEiBlend.buf").remap(vgRemap, fixedBlendFile = "PuppetEiGotRemapped.buf")


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

        vgRemap = AGR.ModTypes.Raiden.value.getVGRemap("RaidenBoss")
        fixedBytes = AGR.BlendFile(inputBytes).remap(vgRemap)
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
            |      |
            |      +--> folder
            |      |      |
            |      |      +--> folderInFolder
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree2.buf
            |      |
            |      +--> folder2
            |      |      |
            |      |      +--> folderInFolder2
            |      |             |
            |      |             +--> AnotherFolder
            |      |                    |
            |      |                    +--> disconnectedSubTree2.ini
            |      |
            |      +--> pythonScript
            |      |      |
            |      |      +--> Run
            |      |             |
            |      |             +--> example.py
            |      |
            |      +--> ei.ini
            |      |
            |      +--> ei2.ini
            |      |
            |      +--> RaidenShogunBlend.buf
            |
            +--> ParentNodeBlend.buf


    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf


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
            hash = 1a495487
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
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf


    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
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

        fixService = AGR.RemapServiceCLI(path = "../../", verbose = False, keepBackups = False)
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Contains the fixed files for the mods.

    .. dropdown:: File Structure
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
            |      |
            |      +--> folder
            |      |      |
            |      |      +--> folderInFolder
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree2.buf
            |      |             |
            |      |             +--> RaidenBossRemapBlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> RaidenBossRemapBlendToDisconnectedSubTree2.buf
            |      |
            |      +--> folder2
            |      |      |
            |      |      +--> folderInFolder2
            |      |             |
            |      |             +--> AnotherFolder
            |      |                    |
            |      |                    +--> disconnectedSubTree2.ini
            |      |
            |      +--> pythonScript
            |      |      |
            |      |      +--> Run
            |      |             |
            |      |             +--> example.py
            |      |
            |      +--> ei.ini
            |      |
            |      +--> ei2.ini
            |      |
            |      +--> RaidenShogunBlend.buf
            |      |
            |      +--> RaidenShogunRaidenBossRemapBlend.buf
            |
            +--> ParentNodeBlend.buf
            |
            +--> ParentNodeRaidenBossRemapBlend.buf


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\Mod\folder\folderInFolder\RaidenBossRemapBlendToDisconnectedSubTree.buf

            ; --------------------------------------------


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
            hash = 1a495487
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
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
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
            filename = ..\ParentNodeRaidenBossRemapBlend.buf

            ; --------------------------------------------


    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunRaidenBossRemapBlend.buf

            ; --------------------------------------------


    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\..\folder\folderInFolder\RaidenBossRemapBlendToDisconnectedSubTree2.buf

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

    .. dropdown:: File Structure
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
            |      |
            |      +--> folder
            |      |      |
            |      |      +--> folderInFolder
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree2.buf
            |      |             |
            |      |             +--> RaidenBossRemapBlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> RaidenBossRemapBlendToDisconnectedSubTree2.buf
            |      |
            |      +--> folder2
            |      |      |
            |      |      +--> folderInFolder2
            |      |             |
            |      |             +--> AnotherFolder
            |      |                    |
            |      |                    +--> disconnectedSubTree2.ini
            |      |
            |      +--> pythonScript
            |      |      |
            |      |      +--> Run
            |      |             |
            |      |             +--> example.py
            |      |
            |      +--> ei.ini
            |      |
            |      +--> ei2.ini
            |      |
            |      +--> RaidenShogunBlend.buf
            |      |
            |      +--> RaidenShogunRaidenBossRemapBlend.buf
            |
            +--> ParentNodeBlend.buf
            |
            +--> ParentNodeRaidenBossRemapBlend.buf


    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\Mod\folder\folderInFolder\RaidenBossRemapBlendToDisconnectedSubTree.buf

            ; --------------------------------------------


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
            hash = 1a495487
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
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
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
            filename = ..\ParentNodeRaidenBossRemapBlend.buf

            ; --------------------------------------------


    .. dropdown:: ei2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: ei2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunRaidenBossRemapBlend.buf

            ; --------------------------------------------


    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../../folder/folderInFolder/BlendToDisconnectedSubTree2.buf

            ; --------------- Raiden Remap ---------------
            ; Raiden remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Raiden mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRaidenShogunRaidenBossRemapBlend]
            hash = fe5c0180
            vb1 = ResourceRaidenShogunRaidenBossRemapBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunRaidenBossRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\..\..\folder\folderInFolder\RaidenBossRemapBlendToDisconnectedSubTree2.buf

            ; --------------------------------------------



.. dropdown:: Code
    :open:
    :animate: fade-in-slide-down

    .. code-block:: python
        :caption: example.py
        :linenos:

        import AnimeGameRemap as AGR

        fixService = AGR.RemapServiceCLI(path = "../../", verbose = False, keepBackups = False, undoOnly = True)
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
            |      |
            |      +--> folder
            |      |      |
            |      |      +--> folderInFolder
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree.buf
            |      |             |
            |      |             +--> BlendToDisconnectedSubTree2.buf
            |      |
            |      +--> folder2
            |      |      |
            |      |      +--> folderInFolder2
            |      |             |
            |      |             +--> AnotherFolder
            |      |                    |
            |      |                    +--> disconnectedSubTree2.ini
            |      |
            |      +--> pythonScript
            |      |      |
            |      |      +--> Run
            |      |             |
            |      |             +--> example.py
            |      |
            |      +--> ei.ini
            |      |
            |      +--> ei2.ini
            |      |
            |      +--> RaidenShogunBlend.buf
            |
            +--> ParentNodeBlend.buf


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: disconnectedSubTree.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = ../../Mod/folder/folderInFolder/BlendToDisconnectedSubTree.buf


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
            hash = 1a495487
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
            hash = 1a495487
            vb1 = ResourceRaidenShogunBlend
            handling = skip
            draw = 21916,0

            [ResourceRaidenShogunBlend]
            type = Buffer
            stride = 32
            filename = RaidenShogunBlend.buf


    .. dropdown:: disconnectedSubTree2.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: disconnectedSubTree2.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
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
    An override registered through :class:`FixRaidenBoss2.CppStrategyOverrides` changes how the software fixes that character everywhere in
    your program, not just for the one fix shown in the example below. Call :meth:`FixRaidenBoss2.CppStrategyOverrides.clear` once you
    are done, so later fixes go back to the default implementation.

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
            +--> KiraraBlend.buf
            |
            +--> Neko.dds


    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: KiraraAlt.ini
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

        RegRef = AGR.GIMICharFixerConfig.RegRef
        RegValChecks = AGR.GIMICharFixerConfig.RegValChecks
        TexEdit = AGR.GIMICharFixerConfig.TexEdit

        ORFix = AGR.IniKeywords.ORFixPath.value
        NNFix = r"CommandList\global\ORFix\NNFix"
        TexFx = r"CommandList\TexFx\TN.0"


        # Edit Kirara's body so that her body's skin tone matches with her face
        #
        # -- Notes --:
        # If you do not like how we edit her body, you can play around with her BodyDiffuse.dds or her BodyLightMap.dds
        #   in your favourite image editor (Paint.net, Photoshop, etc...) or you can tweak the code below
        #
        # A filter is given the texture itself, so it edits the texture in place
        def darkenDiffuse(texFile):
            AGR.GammaFilter(AGR.ColourConsts.SRGBGamma.value).transform(texFile)

        def makeFaceOpaque(texFile):
            pixels = bytearray(texFile.getPixels())
            pixels[3::4] = bytes([1]) * (len(pixels) // 4)
            texFile.setPixels(bytes(pixels), texFile.width, texFile.height)

        def reflectionKeys(obj: str):
            return [f"ResourceRef{obj}Diffuse", f"ResourceRef{obj}LightMap", "$CharacterIB"]


        # ==== Override how Kirara is fixed ======

        config = AGR.GIMICharFixerConfig()
        config.drawnObjs = ["head", "body", "dress"]

        # Kirara's body is drawn a second time as part of KiraraBoots' head, while KiraraBoots' own body is hidden
        config.objSplits = [("head", ["head"]), ("body", ["body", "head"]), ("dress", ["dress"])]
        config.objNewRegVals = [("body", [("ib", "null")])]

        # only darken the copy of Kirara's body that is drawn as KiraraBoots' head
        config.texEdits = [TexEdit("head", "ps-t1", "DarkenDiffuse", darkenDiffuse, srcObj = "body"),
                           TexEdit("face", "ps-t0", "OpaqueFaceDiffuse", makeFaceOpaque, toReg = "ps-t1")]

        # ---- the rest is the same as the default fix ----
        config.objRegRemovals = [("head", reflectionKeys("Head")), ("body", reflectionKeys("Body")), ("dress", reflectionKeys("Dress"))]
        config.objRegRemaps = [("dress", [("ps-t1", [RegRef("ps-t0", RegValChecks.isDiffuse)], True),
                                          ("ps-t2", [RegRef("ps-t1", RegValChecks.isLightMap)], True)])]
        config.objFixCalls = [("head", [ORFix, TexFx]), ("body", [ORFix, TexFx]), ("dress", [NNFix, TexFx])]

        AGR.CppStrategyOverrides.setFixer("Kirara", "KiraraBoots", AGR.makeGIMICharFixer(config))

        # ========================================

        # fix the mod
        remapService = AGR.RemapServiceCLI(verbose = False, keepBackups = False)
        remapService.fix()

        # go back to the default fix
        AGR.CppStrategyOverrides.clear()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the alternative fix for Kirara applied

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
            +--> KiraraBlend.buf
            |
            +--> KiraraBootsBodyRemapTexDsa GG3.dds
            |
            +--> KiraraBootsFaceRemapTexPoj JcH.dds
            |
            +--> KiraraFaceDiffuseRemapDL.dds
            |
            +--> KiraraKiraraBootsRemapBlend.buf
            |
            +--> Neko.dds


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: KiraraAlt.ini
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
            ; Kirara remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Kirara mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [ResourceKiraraFaceDiffuseRemapDL]
            filename = KiraraFaceDiffuseRemapDL.dds

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
            ps-t1 = ResourceKiraraBodyDiffuse
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraDressKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 80295
            ib = null
            ps-t0 = ResourceKiraraDressNormalMap
            ps-t0 = ResourceKiraraDressDiffuse
            ps-t1 = ResourceKiraraDressLightMap
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideKiraraKiraraBootsRemapIB]
            hash = 846979e2
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraKiraraBootsRemapBlend]
            hash = 53a2502b
            vb1 = ResourceKiraraKiraraBootsRemapBlend
            handling = skip
            draw = 41553,0

            [TextureOverrideKiraraKiraraBootsRemapPosition]
            hash = f8013ba9
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraKiraraBootsRemapTexcoord]
            hash = 596e8fe0
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaiseKiraraBootsRemapFix]
            hash = 4955fc99

            [TextureOverrideKiraraFaceKiraraBootsRemapFix]
            hash = 6eb20522
            ps-t1 = ResourceKiraraFaceDiffuseRemapDL
            ps-t0 = ResourceKiraraFaceDiffuseRemapDLKiraraBootsOpaqueFaceDiffuseRemapTex

            [ResourceKiraraKiraraBootsRemapBlend]
            type = Buffer
            stride = 32
            filename = KiraraKiraraBootsRemapBlend.buf

            [ResourceKiraraFaceDiffuseRemapDLKiraraBootsOpaqueFaceDiffuseRemapTex]
            filename = KiraraBootsFaceRemapTexPoj JcH.dds

            ; --------------------------------------------


    .. dropdown:: KiraraAltRemapFix1.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: KiraraAltRemapFix1.ini
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
            ; Kirara remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Kirara mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideKiraraHeadKiraraBootsRemapFix]
            hash = 846979e2
            match_first_index = 0
            ib = ResourceKiraraBodyIB
            ps-t0 = ResourceKiraraBodyNormalMap
            ps-t1 = ResourceKiraraBodyDiffuseKiraraBootsDarkenDiffuseRemapTex
            ps-t2 = ResourceKiraraBodyLightMap
            run = CommandList\global\ORFix\ORFix

            [TextureOverrideKiraraKiraraBootsRemapIB]
            hash = 846979e2
            handling = skip
            drawindexed = auto

            [TextureOverrideKiraraKiraraBootsRemapBlend]
            hash = 53a2502b
            vb1 = ResourceKiraraKiraraBootsRemapBlend
            handling = skip
            draw = 41553,0

            [TextureOverrideKiraraKiraraBootsRemapPosition]
            hash = f8013ba9
            vb0 = ResourceKiraraPosition

            [TextureOverrideKiraraKiraraBootsRemapTexcoord]
            hash = 596e8fe0
            vb1 = ResourceKiraraTexcoord

            [TextureOverrideKiraraVertexLimitRaiseKiraraBootsRemapFix]
            hash = 4955fc99

            [TextureOverrideKiraraFaceKiraraBootsRemapFix]
            hash = 6eb20522
            ps-t1 = ResourceKiraraFaceDiffuseRemapDL
            ps-t0 = ResourceKiraraFaceDiffuseRemapDLKiraraBootsOpaqueFaceDiffuseRemapTex

            [ResourceKiraraKiraraBootsRemapBlend]
            type = Buffer
            stride = 32
            filename = KiraraKiraraBootsRemapBlend.buf

            [ResourceKiraraBodyDiffuseKiraraBootsDarkenDiffuseRemapTex]
            filename = KiraraBootsBodyRemapTexDsa GG3.dds

            [ResourceKiraraFaceDiffuseRemapDLKiraraBootsOpaqueFaceDiffuseRemapTex]
            filename = KiraraBootsFaceRemapTexPoj JcH.dds

            ; --------------------------------------------



:raw-html:`<br />`

Forcibly remap a mod to a different character
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. caution::
    Sometimes, the software may possibly identify the wrong character for a mod and generate the wrong remap for the mod.
    For this case, you may want to forcibly tell the software the character for a specific mod, to make the correct remap.

:raw-html:`<br />`

The example below shows how to forcibly use the strategy for remapping Rosaria onto a Kirara mod.

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
            +--> KiraraBlend.buf
            |
            +--> Neko.dds


    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: KiraraAlt.ini
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

        fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, forcedType = "rosaria")
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the fix for Rosaria applied onto the Kirara mod

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
            +--> KiraraBlend.buf
            |
            +--> Neko.dds
            |
            +--> RosariaBlendRemapDL.buf
            |
            +--> RosariaBodyDiffuseRemapDL.dds
            |
            +--> RosariaBodyLightMapRemapDL.dds
            |
            +--> RosariaBodyRemapDL.ib
            |
            +--> RosariaDressDiffuseRemapDL.dds
            |
            +--> RosariaDressLightMapRemapDL.dds
            |
            +--> RosariaDressRemapDL.ib
            |
            +--> RosariaExtraDiffuseRemapDL.dds
            |
            +--> RosariaExtraLightMapRemapDL.dds
            |
            +--> RosariaExtraRemapDL.ib
            |
            +--> RosariaFaceDiffuseRemapDL.dds
            |
            +--> RosariaHeadDiffuseRemapDL.dds
            |
            +--> RosariaHeadLightMapRemapDL.dds
            |
            +--> RosariaHeadRemapDL.ib
            |
            +--> RosariaPositionRemapDL.buf
            |
            +--> RosariaRosariaCNRemapBlendRemapDL.buf
            |
            +--> RosariaTexcoordRemapDL.buf


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: KiraraAlt.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: KiraraAlt.ini
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

            ; --------------- Rosaria Remap ---------------
            ; Rosaria remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Rosaria mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideRosariaHeadRosariaCNRemapFix]
            ib = ResourceRosariaHeadIbRemapDL
            ps-t1 = ResourceRosariaHeadLightMapRemapDL
            hash = bdca273e
            match_first_index = 0
            ps-t0 = ResourceRosariaHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideRosariaBodyRosariaCNRemapFix]
            ib = ResourceRosariaBodyIbRemapDL
            ps-t1 = ResourceRosariaBodyLightMapRemapDL
            hash = bdca273e
            match_first_index = 11025
            ps-t0 = ResourceRosariaBodyDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideRosariaDressRosariaCNRemapFix]
            ib = ResourceRosariaDressIbRemapDL
            ps-t1 = ResourceRosariaDressLightMapRemapDL
            hash = bdca273e
            match_first_index = 46539
            ps-t0 = ResourceRosariaDressDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideRosariaExtraRosariaCNRemapFix]
            ib = ResourceRosariaExtraIbRemapDL
            ps-t1 = ResourceRosariaExtraLightMapRemapDL
            hash = bdca273e
            match_first_index = 48441
            ps-t0 = ResourceRosariaExtraDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideRosariaRosariaCNRemapBlendRemapFix]
            hash = a7bee046
            vb1 = ResourceRosariaRosariaCNRemapBlendRemapDL

            [TextureOverrideRosariaRosariaCNRemapPositionRemapFix]
            hash = 59a1f8b1
            vb0 = ResourceRosariaPositionRemapDL

            [TextureOverrideRosariaRosariaCNRemapTexcoordRemapFix]
            hash = 86e0d16b
            vb1 = ResourceRosariaTexcoordRemapDL

            [TextureOverrideRosariaFaceRosariaCNRemapFix]
            hash = 2abd61ee
            ps-t1 = ResourceRosariaFaceDiffuseRemapDL

            [ResourceRosariaHeadDiffuseRemapDL]
            filename = RosariaHeadDiffuseRemapDL.dds

            [ResourceRosariaHeadLightMapRemapDL]
            filename = RosariaHeadLightMapRemapDL.dds

            [ResourceRosariaHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = RosariaHeadRemapDL.ib

            [ResourceRosariaBodyDiffuseRemapDL]
            filename = RosariaBodyDiffuseRemapDL.dds

            [ResourceRosariaBodyLightMapRemapDL]
            filename = RosariaBodyLightMapRemapDL.dds

            [ResourceRosariaBodyIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = RosariaBodyRemapDL.ib

            [ResourceRosariaDressDiffuseRemapDL]
            filename = RosariaDressDiffuseRemapDL.dds

            [ResourceRosariaDressLightMapRemapDL]
            filename = RosariaDressLightMapRemapDL.dds

            [ResourceRosariaDressIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = RosariaDressRemapDL.ib

            [ResourceRosariaExtraDiffuseRemapDL]
            filename = RosariaExtraDiffuseRemapDL.dds

            [ResourceRosariaExtraLightMapRemapDL]
            filename = RosariaExtraLightMapRemapDL.dds

            [ResourceRosariaExtraIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = RosariaExtraRemapDL.ib

            [ResourceRosariaFaceDiffuseRemapDL]
            filename = RosariaFaceDiffuseRemapDL.dds

            [ResourceRosariaBlendRemapDL]
            type = Buffer
            stride = 32
            filename = RosariaBlendRemapDL.buf

            [ResourceRosariaPositionRemapDL]
            type = Buffer
            stride = 40
            filename = RosariaPositionRemapDL.buf

            [ResourceRosariaTexcoordRemapDL]
            type = Buffer
            stride = 20
            filename = RosariaTexcoordRemapDL.buf

            [ResourceRosariaRosariaCNRemapBlendRemapDL]
            type = Buffer
            stride = 32
            filename = RosariaRosariaCNRemapBlendRemapDL.buf

            ; ---------------------------------------------



:raw-html:`<br />`


Remap Only a Few Selected Characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example, by running the program called `example.py`, the fix will only fix the mods for Keqing, Jean and Amber.
Mods for Shenhe and Raiden will not be fixed.

.. note::
    You can enter the nicknames/aliases of a mod in upper/lower case instead of just the regular name of the mod.
    Please refer to :ref:`Mod Types <commandOpts:Mod Types>` for the available aliases for each mod.

:raw-html:`<br />`

.. dropdown:: Input
    :animate: fade-in-slide-down

    Assume we have this file structure:

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 67

            Mods
            |
            +--> Amber
            |      |
            |      +--> Amber.ini
            |      |
            |      +--> AmberBlend.buf
            |
            +--> Jean
            |      |
            |      +--> CuteJean
            |      |      |
            |      |      +--> CuteJean.ini
            |      |
            |      +--> SmolJean
            |      |      |
            |      |      +--> CuteJeanBlend.buf
            |      |      |
            |      |      +--> SmolJean.ini
            |      |
            |      +--> merged.ini
            |      |
            |      +--> SmolJeanBlend.buf
            |      |
            |      +--> SmollerJean.dds
            |
            +--> Kequeen
            |      |
            |      +--> Buffs
            |      |      |
            |      |      +--> ISwearItsFor.buf
            |      |      |
            |      |      +--> SmallerHitboxes.buf
            |      |
            |      +--> IniOrJoJ
            |             |
            |             +--> BestGurl.ini
            |             |
            |             +--> CatGirl.dds
            |             |
            |             +--> Cutesy.dds
            |             |
            |             +--> Cutie.ini
            |             |
            |             +--> CutiePie.dds
            |             |
            |             +--> Patootie.dds
            |
            +--> Raiden
            |      |
            |      +--> KindOfGettingTired.buf
            |      |
            |      +--> WritingTheseTestCases.ini
            |
            +--> Yasu
            |      |
            |      +--> DaGreatEqualizerIsTheDes.buf
            |      |
            |      +--> DesDesDesDesDesDes.buf
            |      |
            |      +--> DieDaDes.buf
            |      |
            |      +--> Endless9999GoldenTruth.ini
            |      |
            |      +--> SentenceToDes.buf
            |
            +--> example.py


    :raw-html:`<br />`

    Assume below is the content of the .ini files

    .. dropdown:: Amber.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Amber.ini
            :linenos:

            [TextureOverrideAmberBlend]
            hash = ca5bd26e
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
            hash = 3cb8153c
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
            hash = 3cb8153c
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

            [ResourceJeanSeaBodyLightMap]
            filename = ../SmollerJean.dds


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
            hash = 3cb8153c
            if $swapvar == 0
                vb1 = ResourceJeanBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceJeanSeaBodyLightMap]
            filename = SmollerJean.dds

            [ResourceJeanBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanBlend.buf

            [ResourceJeanBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanBlend.buf


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
            hash = 0bf8e621
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
            hash = 0bf8e621
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


    .. dropdown:: WritingTheseTestCases.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: WritingTheseTestCases.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
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
            hash = 541cf273
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

        fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["kequeen", "aMbEr", "ACTINGGRANDMASTER"])
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with only the mods for Keqing, Jean and Amber fixed

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 167

            Mods
            |
            +--> Amber
            |      |
            |      +--> Amber.ini
            |      |
            |      +--> AmberAmberCNRemapBlend.buf
            |      |
            |      +--> AmberBlend.buf
            |      |
            |      +--> AmberBodyDiffuseRemapDL.dds
            |      |
            |      +--> AmberBodyLightMapRemapDL.dds
            |      |
            |      +--> AmberBodyRemapDL.ib
            |      |
            |      +--> AmberFaceDiffuseRemapDL.dds
            |      |
            |      +--> AmberHeadDiffuseRemapDL.dds
            |      |
            |      +--> AmberHeadLightMapRemapDL.dds
            |      |
            |      +--> AmberHeadRemapDL.ib
            |      |
            |      +--> AmberPositionRemapDL.buf
            |      |
            |      +--> AmberTexcoordRemapDL.buf
            |
            +--> Jean
            |      |
            |      +--> CuteJean
            |      |      |
            |      |      +--> CuteJean.ini
            |      |      |
            |      |      +--> JeanBodyDiffuseRemapDL.dds
            |      |      |
            |      |      +--> JeanBodyLightMapRemapDL.dds
            |      |      |
            |      |      +--> JeanBodyRemapDL.ib
            |      |      |
            |      |      +--> JeanFaceDiffuseRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadDiffuseRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadLightMapRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadRemapDL.ib
            |      |      |
            |      |      +--> JeanPositionRemapDL.buf
            |      |      |
            |      |      +--> JeanSeaBodyRemapTexO65 DK+.dds
            |      |      |
            |      |      +--> JeanTexcoordRemapDL.buf
            |      |
            |      +--> SmolJean
            |      |      |
            |      |      +--> CuteJeanBlend.buf
            |      |      |
            |      |      +--> CuteJeanJeanCNRemapBlend.buf
            |      |      |
            |      |      +--> CuteJeanJeanSeaRemapBlend.buf
            |      |      |
            |      |      +--> JeanFaceDiffuseRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadDiffuseRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadLightMapRemapDL.dds
            |      |      |
            |      |      +--> JeanHeadRemapDL.ib
            |      |      |
            |      |      +--> JeanPositionRemapDL.buf
            |      |      |
            |      |      +--> JeanTexcoordRemapDL.buf
            |      |      |
            |      |      +--> SmolJean.ini
            |      |
            |      +--> JeanFaceDiffuseRemapDL.dds
            |      |
            |      +--> JeanHeadDiffuseRemapDL.dds
            |      |
            |      +--> JeanHeadLightMapRemapDL.dds
            |      |
            |      +--> JeanHeadRemapDL.ib
            |      |
            |      +--> JeanPositionRemapDL.buf
            |      |
            |      +--> JeanSeaBodyRemapTexOKA DK+.dds
            |      |
            |      +--> JeanTexcoordRemapDL.buf
            |      |
            |      +--> merged.ini
            |      |
            |      +--> SmolJeanBlend.buf
            |      |
            |      +--> SmolJeanJeanCNRemapBlend.buf
            |      |
            |      +--> SmolJeanJeanSeaRemapBlend.buf
            |      |
            |      +--> SmollerJean.dds
            |
            +--> Kequeen
            |      |
            |      +--> Buffs
            |      |      |
            |      |      +--> ISwearItsFor.buf
            |      |      |
            |      |      +--> ISwearItsForKeqingOpulentRemapBlend.buf
            |      |      |
            |      |      +--> SmallerHitboxes.buf
            |      |      |
            |      |      +--> SmallerHitboxesKeqingOpulentRemapBlend.buf
            |      |
            |      +--> IniOrJoJ
            |             |
            |             +--> BestGurl.ini
            |             |
            |             +--> BestGurlRemapFix1.ini
            |             |
            |             +--> CatGirl.dds
            |             |
            |             +--> Cutesy.dds
            |             |
            |             +--> Cutie.ini
            |             |
            |             +--> CutiePie.dds
            |             |
            |             +--> CutieRemapFix1.ini
            |             |
            |             +--> KeqingFaceDiffuseRemapDL.dds
            |             |
            |             +--> KeqingHeadDiffuseRemapDL.dds
            |             |
            |             +--> KeqingHeadLightMapRemapDL.dds
            |             |
            |             +--> KeqingHeadRemapDL.ib
            |             |
            |             +--> KeqingOpulentHeadRemapTexBkA J93.dds
            |             |
            |             +--> KeqingOpulentHeadRemapTexCgN J93.dds
            |             |
            |             +--> KeqingOpulentHeadRemapTexKNs J93.dds
            |             |
            |             +--> KeqingPositionRemapDL.buf
            |             |
            |             +--> KeqingTexcoordRemapDL.buf
            |             |
            |             +--> Patootie.dds
            |
            +--> Raiden
            |      |
            |      +--> KindOfGettingTired.buf
            |      |
            |      +--> WritingTheseTestCases.ini
            |
            +--> Yasu
            |      |
            |      +--> DaGreatEqualizerIsTheDes.buf
            |      |
            |      +--> DesDesDesDesDesDes.buf
            |      |
            |      +--> DieDaDes.buf
            |      |
            |      +--> Endless9999GoldenTruth.ini
            |      |
            |      +--> SentenceToDes.buf
            |
            +--> example.py


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: Amber.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: Amber.ini
            :linenos:

            [TextureOverrideAmberBlend]
            hash = ca5bd26e
            vb1 = ResourceAmberBlend
            handling = skip
            draw = 21916,0

            [ResourceAmberBlend]
            type = Buffer
            stride = 32
            filename = AmberBlend.buf

            ; --------------- Amber Remap ---------------
            ; Amber remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Amber mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideAmberHeadAmberCNRemapFix]
            ib = ResourceAmberHeadIbRemapDL
            ps-t1 = ResourceAmberHeadLightMapRemapDL
            hash = b41d4d94
            match_first_index = 0
            ps-t0 = ResourceAmberHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix
            drawindexed = auto

            [TextureOverrideAmberBodyAmberCNRemapFix]
            ib = ResourceAmberBodyIbRemapDL
            ps-t1 = ResourceAmberBodyLightMapRemapDL
            hash = b41d4d94
            match_first_index = 5670
            ps-t0 = ResourceAmberBodyDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix
            drawindexed = auto

            [TextureOverrideAmberAmberCNRemapBlend]
            hash = f35340d5
            vb1 = ResourceAmberAmberCNRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideAmberAmberCNRemapPositionRemapFix]
            hash = 557b2eff
            vb0 = ResourceAmberPositionRemapDL

            [TextureOverrideAmberAmberCNRemapTexcoordRemapFix]
            hash = dbc594b6
            vb1 = ResourceAmberTexcoordRemapDL

            [TextureOverrideAmberFaceAmberCNRemapFix]
            hash = 1d064079
            ps-t1 = ResourceAmberFaceDiffuseRemapDL

            [ResourceAmberHeadDiffuseRemapDL]
            filename = AmberHeadDiffuseRemapDL.dds

            [ResourceAmberHeadLightMapRemapDL]
            filename = AmberHeadLightMapRemapDL.dds

            [ResourceAmberHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = AmberHeadRemapDL.ib

            [ResourceAmberBodyDiffuseRemapDL]
            filename = AmberBodyDiffuseRemapDL.dds

            [ResourceAmberBodyLightMapRemapDL]
            filename = AmberBodyLightMapRemapDL.dds

            [ResourceAmberBodyIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = AmberBodyRemapDL.ib

            [ResourceAmberFaceDiffuseRemapDL]
            filename = AmberFaceDiffuseRemapDL.dds

            [ResourceAmberPositionRemapDL]
            type = Buffer
            stride = 40
            filename = AmberPositionRemapDL.buf

            [ResourceAmberTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = AmberTexcoordRemapDL.buf

            [ResourceAmberAmberCNRemapBlend]
            type = Buffer
            stride = 32
            filename = AmberAmberCNRemapBlend.buf

            ; -------------------------------------------


    .. dropdown:: CuteJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: CuteJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            hash = 3cb8153c
            vb1 = ResourceJeanBlend
            handling = skip
            draw = 21916,0

            [ResourceJeanBlend]
            type = Buffer
            stride = 32
            filename = ../SmolJean/CuteJeanBlend.buf

            ; --------------- Jean Remap ---------------
            ; Jean remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Jean mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanHeadJeanCNRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = aad861e0
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanCNRemapFix]
            ib = ResourceJeanBodyIbRemapDL
            ps-t1 = ResourceJeanBodyLightMapRemapDL
            hash = aad861e0
            match_first_index = 7779
            ps-t0 = ResourceJeanBodyDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanJeanCNRemapBlend]
            hash = d159bf31
            vb1 = ResourceJeanJeanCNRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanJeanCNRemapPositionRemapFix]
            hash = 93bb2522
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanCNRemapTexcoordRemapFix]
            hash = 0ffefb98
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanCNRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanBodyDiffuseRemapDL]
            filename = JeanBodyDiffuseRemapDL.dds

            [ResourceJeanBodyLightMapRemapDL]
            filename = JeanBodyLightMapRemapDL.dds

            [ResourceJeanBodyIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanBodyRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [ResourceJeanJeanCNRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\SmolJean\CuteJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            hash = ac801371
            vb1 = ResourceJeanJeanSeaRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanJeanSeaRemapPositionRemapFix]
            hash = 16fef1eb
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanSeaRemapTexcoordRemapFix]
            hash = 3ffb0363
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanSeaRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanBodyDiffuseRemapDL]
            filename = JeanBodyDiffuseRemapDL.dds

            [ResourceJeanBodyLightMapRemapDL]
            filename = JeanBodyLightMapRemapDL.dds

            [ResourceJeanBodyIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanBodyRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [TextureOverrideJeanHeadJeanSeaRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = 69c0c24e
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanSeaRemapFix]
            ib = ResourceJeanBodyIbRemapDL
            ps-t1 = ResourceJeanBodyLightMapRemapDLJeanSeaShadeLightMapRemapTex
            hash = 69c0c24e
            match_first_index = 7662
            ps-t0 = ResourceJeanBodyDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanDressJeanSeaRemapFix]
            ib = null
            ps-t1 = ResourceJeanBodyLightMapRemapDL
            hash = 69c0c24e
            match_first_index = 52542
            ps-t0 = ResourceJeanBodyDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [ResourceJeanJeanSeaRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\SmolJean\CuteJeanJeanSeaRemapBlend.buf

            [ResourceJeanBodyLightMapRemapDLJeanSeaShadeLightMapRemapTex]
            filename = JeanSeaBodyRemapTexO65 DK+.dds

            ; *******************

            ; ------------------------------------------


    .. dropdown:: SmolJean.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: SmolJean.ini
            :linenos:

            [TextureOverrideJeanBlend]
            hash = 3cb8153c
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

            [ResourceJeanSeaBodyLightMap]
            filename = ../SmollerJean.dds

            ; --------------- Jean Remap ---------------
            ; Jean remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Jean mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanHeadJeanCNRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = aad861e0
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanCNRemapFix]
            hash = aad861e0
            match_first_index = 7779
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanJeanCNRemapBlend]
            hash = d159bf31
            vb1 = ResourceJeanJeanCNRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanJeanCNRemapPositionRemapFix]
            hash = 93bb2522
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanCNRemapTexcoordRemapFix]
            hash = 0ffefb98
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanCNRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [ResourceJeanJeanCNRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\SmolJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            hash = ac801371
            vb1 = ResourceJeanJeanSeaRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideJeanJeanSeaRemapPositionRemapFix]
            hash = 16fef1eb
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanSeaRemapTexcoordRemapFix]
            hash = 3ffb0363
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanSeaRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [TextureOverrideJeanHeadJeanSeaRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = 69c0c24e
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanSeaRemapFix]
            hash = 69c0c24e
            match_first_index = 7662
            ib = ResourceJeanSeaBodyIB
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMapJeanSeaShadeLightMapRemapTex
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanDressJeanSeaRemapFix]
            hash = 69c0c24e
            match_first_index = 52542
            ib = null
            ps-t0 = ResourceJeanSeaBodyDiffuse
            ps-t1 = ResourceJeanSeaBodyLightMap
            run = CommandList\global\ORFix\NNFix

            [ResourceJeanJeanSeaRemapBlend]
            type = Buffer
            stride = 32
            filename = ..\SmolJeanJeanSeaRemapBlend.buf

            [ResourceJeanSeaBodyLightMapJeanSeaShadeLightMapRemapTex]
            filename = ..\JeanSeaBodyRemapTexOKA DK+.dds

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
            hash = 3cb8153c
            if $swapvar == 0
                vb1 = ResourceJeanBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanBlend.1
                handling = skip
                draw = 21916,0
            endif

            [ResourceJeanSeaBodyLightMap]
            filename = SmollerJean.dds

            [ResourceJeanBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanBlend.buf

            [ResourceJeanBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean/CuteJeanBlend.buf

            ; --------------- Jean Remap ---------------
            ; Jean remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Jean mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            ; ***** JeanCN *****
            [TextureOverrideJeanHeadJeanCNRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = aad861e0
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanCNRemapFix]
            if $swapvar == 0
                hash = aad861e0
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
                run = CommandList\global\ORFix\NNFix
            else if $swapvar == 1
                hash = aad861e0
                match_first_index = 7779
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
                run = CommandList\global\ORFix\NNFix
            endif

            [TextureOverrideJeanJeanCNRemapBlend]
            hash = d159bf31
            if $swapvar == 0
                vb1 = ResourceJeanJeanCNRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanJeanCNRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideJeanJeanCNRemapPositionRemapFix]
            hash = 93bb2522
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanCNRemapTexcoordRemapFix]
            hash = 0ffefb98
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanCNRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [ResourceJeanJeanCNRemapBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanJeanCNRemapBlend.buf

            [ResourceJeanJeanCNRemapBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean\CuteJeanJeanCNRemapBlend.buf

            ; ******************

            ; ***** JeanSea *****
            [TextureOverrideJeanJeanSeaRemapBlend]
            hash = ac801371
            if $swapvar == 0
                vb1 = ResourceJeanJeanSeaRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceJeanJeanSeaRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideJeanJeanSeaRemapPositionRemapFix]
            hash = 16fef1eb
            vb0 = ResourceJeanPositionRemapDL

            [TextureOverrideJeanJeanSeaRemapTexcoordRemapFix]
            hash = 3ffb0363
            vb1 = ResourceJeanTexcoordRemapDL

            [TextureOverrideJeanFaceJeanSeaRemapFix]
            hash = c2d1a57e
            ps-t1 = ResourceJeanFaceDiffuseRemapDL

            [ResourceJeanHeadDiffuseRemapDL]
            filename = JeanHeadDiffuseRemapDL.dds

            [ResourceJeanHeadLightMapRemapDL]
            filename = JeanHeadLightMapRemapDL.dds

            [ResourceJeanHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = JeanHeadRemapDL.ib

            [ResourceJeanFaceDiffuseRemapDL]
            filename = JeanFaceDiffuseRemapDL.dds

            [ResourceJeanPositionRemapDL]
            type = Buffer
            stride = 40
            filename = JeanPositionRemapDL.buf

            [ResourceJeanTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = JeanTexcoordRemapDL.buf

            [TextureOverrideJeanHeadJeanSeaRemapFix]
            ib = ResourceJeanHeadIbRemapDL
            ps-t1 = ResourceJeanHeadLightMapRemapDL
            hash = 69c0c24e
            match_first_index = 0
            ps-t0 = ResourceJeanHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideJeanBodyJeanSeaRemapFix]
            if $swapvar == 0
                hash = 69c0c24e
                match_first_index = 7662
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMapJeanSeaShadeLightMapRemapTex
                run = CommandList\global\ORFix\NNFix
            else if $swapvar == 1
                hash = 69c0c24e
                match_first_index = 7662
                ib = ResourceJeanSeaBodyIB
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMapJeanSeaShadeLightMapRemapTex
                run = CommandList\global\ORFix\NNFix
            endif

            [TextureOverrideJeanDressJeanSeaRemapFix]
            if $swapvar == 0
                hash = 69c0c24e
                match_first_index = 52542
                ib = null
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
                run = CommandList\global\ORFix\NNFix
            else if $swapvar == 1
                hash = 69c0c24e
                match_first_index = 52542
                ib = null
                ps-t0 = ResourceJeanSeaBodyDiffuse
                ps-t1 = ResourceJeanSeaBodyLightMap
                run = CommandList\global\ORFix\NNFix
            endif

            [ResourceJeanJeanSeaRemapBlend.0]
            type = Buffer
            stride = 32
            filename = SmolJeanJeanSeaRemapBlend.buf

            [ResourceJeanJeanSeaRemapBlend.1]
            type = Buffer
            stride = 32
            filename = SmolJean\CuteJeanJeanSeaRemapBlend.buf

            [ResourceJeanSeaBodyLightMapJeanSeaShadeLightMapRemapTex]
            filename = JeanSeaBodyRemapTexOKA DK+.dds

            ; *******************

            ; ------------------------------------------


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
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [ResourceKeqingHeadDiffuseRemapDL]
            filename = KeqingHeadDiffuseRemapDL.dds

            [ResourceKeqingHeadLightMapRemapDL]
            filename = KeqingHeadLightMapRemapDL.dds

            [ResourceKeqingHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KeqingHeadRemapDL.ib

            [ResourceKeqingFaceDiffuseRemapDL]
            filename = KeqingFaceDiffuseRemapDL.dds

            [ResourceKeqingPositionRemapDL]
            type = Buffer
            stride = 40
            filename = KeqingPositionRemapDL.buf

            [ResourceKeqingTexcoordRemapDL]
            type = Buffer
            stride = 20
            filename = KeqingTexcoordRemapDL.buf

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3
                ps-t1 = ResourceKeqingDressLightMap.3
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

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0]
            filename = KeqingOpulentHeadRemapTexKNs J93.dds

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3]
            filename = KeqingOpulentHeadRemapTexBkA J93.dds

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
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            ib = ResourceKeqingHeadIbRemapDL
            ps-t1 = ResourceKeqingHeadLightMapRemapDL
            hash = 7c6fc8c3
            match_first_index = 0
            ps-t0 = ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex]
            filename = KeqingOpulentHeadRemapTexCgN J93.dds

            ; --------------------------------------------


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
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [ResourceKeqingHeadDiffuseRemapDL]
            filename = KeqingHeadDiffuseRemapDL.dds

            [ResourceKeqingHeadLightMapRemapDL]
            filename = KeqingHeadLightMapRemapDL.dds

            [ResourceKeqingHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = KeqingHeadRemapDL.ib

            [ResourceKeqingFaceDiffuseRemapDL]
            filename = KeqingFaceDiffuseRemapDL.dds

            [ResourceKeqingPositionRemapDL]
            type = Buffer
            stride = 40
            filename = KeqingPositionRemapDL.buf

            [ResourceKeqingTexcoordRemapDL]
            type = Buffer
            stride = 20
            filename = KeqingTexcoordRemapDL.buf

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            hash = 7c6fc8c3
            match_first_index = 0
            run = CommandListKeqingHeadKeqingOpulentRemapFix

            [CommandListKeqingHeadKeqingOpulentRemapFix]
            if $swapvar == 0
                ib = ResourceKeqingDressIB.0
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0
                ps-t1 = ResourceKeqingDressLightMap.0
                ps-t2 = ResourceKeqingDressMetalMap.0
                ps-t3 = ResourceKeqingDressShadowRamp.0
            else if $swapvar == 1
                ib = ResourceKeqingDressIB.3
                ps-t0 = ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3
                ps-t1 = ResourceKeqingDressLightMap.3
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

            [TextureOverrideKeqingHeadKeqingOpulentBodyRemapFix]
            hash = 7c6fc8c3
            match_first_index = 19623
            run = CommandListKeqingHeadKeqingOpulentBodyRemapFix

            [CommandListKeqingHeadKeqingOpulentBodyRemapFix]
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

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.0]
            filename = KeqingOpulentHeadRemapTexKNs J93.dds

            [ResourceKeqingDressDiffuseKeqingOpulentOpaqueDiffuseRemapTex.3]
            filename = KeqingOpulentHeadRemapTexBkA J93.dds

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
            hash = 0bf8e621
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
            ; Keqing remapped by Albert Gold#2696 and NK#1321. If you used it to remap your Keqing mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideKeqingHeadKeqingOpulentRemapFix]
            ib = ResourceKeqingHeadIbRemapDL
            ps-t1 = ResourceKeqingHeadLightMapRemapDL
            hash = 7c6fc8c3
            match_first_index = 0
            ps-t0 = ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex
            run = CommandList\global\ORFix\NNFix

            [TextureOverrideKeqingKeqingOpulentRemapBlend]
            hash = 6f010b58
            if $swapvar == 0
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.0
                handling = skip
                draw = 21916,0
            else if $swapvar == 1
                vb1 = ResourceKeqingKeqingOpulentRemapBlend.1
                handling = skip
                draw = 21916,0
            endif

            [TextureOverrideKeqingKeqingOpulentRemapPositionRemapFix]
            hash = 0d7e3cc5
            vb0 = ResourceKeqingPositionRemapDL

            [TextureOverrideKeqingKeqingOpulentRemapTexcoordRemapFix]
            hash = 52f78cb7
            vb1 = ResourceKeqingTexcoordRemapDL

            [TextureOverrideKeqingFaceKeqingOpulentRemapFix]
            hash = c2b17f84
            ps-t1 = ResourceKeqingFaceDiffuseRemapDL

            [ResourceKeqingKeqingOpulentRemapBlend.0]
            type = Buffer
            stride = 32
            filename = ..\Buffs\ISwearItsForKeqingOpulentRemapBlend.buf

            [ResourceKeqingKeqingOpulentRemapBlend.1]
            type = Buffer
            stride = 32
            filename = ..\Buffs\SmallerHitboxesKeqingOpulentRemapBlend.buf

            [ResourceKeqingHeadDiffuseRemapDLKeqingOpulentOpaqueDiffuseRemapTex]
            filename = KeqingOpulentHeadRemapTexCgN J93.dds

            ; --------------------------------------------


    .. dropdown:: WritingTheseTestCases.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: WritingTheseTestCases.ini
            :linenos:

            [TextureOverrideRaidenShogunBlend]
            hash = 1a495487
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
            hash = 541cf273
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
    To fix only a .ini file without showing the mod on the original character, go to :ref:`Fixing a .ini File Without Showing the Mod on the Original Character <apiExamples:Fixing a .ini File Without Showing the Mod on the Original Character>`

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
            hash = f35340d5
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

        fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["BaronBunnyCN"], hideOrig = True)
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the mod only shown on the remapped character, and not on the original character

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 21

            AmberCN
            |
            +--> AmberCN.ini
            |
            +--> AmberCNAmberRemapBlend.buf
            |
            +--> AmberCNBlend.buf
            |
            +--> AmberCNFaceDiffuseRemapDL.dds
            |
            +--> AmberCNHeadDiffuseRemapDL.dds
            |
            +--> AmberCNHeadLightMapRemapDL.dds
            |
            +--> AmberCNHeadRemapDL.ib
            |
            +--> AmberCNPositionRemapDL.buf
            |
            +--> AmberCNTexcoordRemapDL.buf
            |
            +--> example.py


    :raw-html:`<br />`

    Below is the new content of the .ini files

    .. dropdown:: AmberCN.ini
        :animate: fade-in-slide-down

        .. code-block:: ini
            :caption: AmberCN.ini
            :linenos:

            ;RemapFixHideOrig -->[TextureOverrideAmberCNBlend]
            ;RemapFixHideOrig -->hash = f35340d5
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
            ; AmberCN remapped by Albert Gold#2696 and NK#1321. If you used it to remap your AmberCN mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideAmberCNHeadAmberRemapFix]
            ib = ResourceAmberCNHeadIbRemapDL
            ps-t1 = ResourceAmberCNHeadLightMapRemapDL
            hash = b03c7e30
            match_first_index = 0
            ps-t0 = ResourceAmberCNHeadDiffuseRemapDL
            run = CommandList\global\ORFix\NNFix
            drawindexed = auto

            [TextureOverrideAmberCNBodyAmberRemapFix]
            hash = b03c7e30
            match_first_index = 5670
            ib = ResourceAmberCNBodyIB
            ps-t0 = ResourceAmberCNBodyDiffuse
            ps-t1 = ResourceAmberCNBodyLightMap
            ps-t2 = ResourceAmberCNBodyMetalMap
            ps-t3 = ResourceAmberCNBodyShadowRamp
            run = CommandList\global\ORFix\NNFix
            drawindexed = auto

            [TextureOverrideAmberCNAmberRemapBlend]
            hash = 36d20a67
            vb1 = ResourceAmberCNAmberRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideAmberCNAmberRemapPositionRemapFix]
            hash = a2ea4b2d
            vb0 = ResourceAmberCNPositionRemapDL

            [TextureOverrideAmberCNAmberRemapTexcoordRemapFix]
            hash = 81b777ca
            vb1 = ResourceAmberCNTexcoordRemapDL

            [TextureOverrideAmberCNFaceAmberRemapFix]
            hash = 1d064079
            ps-t1 = ResourceAmberCNFaceDiffuseRemapDL

            [ResourceAmberCNHeadDiffuseRemapDL]
            filename = AmberCNHeadDiffuseRemapDL.dds

            [ResourceAmberCNHeadLightMapRemapDL]
            filename = AmberCNHeadLightMapRemapDL.dds

            [ResourceAmberCNHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = AmberCNHeadRemapDL.ib

            [ResourceAmberCNFaceDiffuseRemapDL]
            filename = AmberCNFaceDiffuseRemapDL.dds

            [ResourceAmberCNPositionRemapDL]
            type = Buffer
            stride = 40
            filename = AmberCNPositionRemapDL.buf

            [ResourceAmberCNTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = AmberCNTexcoordRemapDL.buf

            [ResourceAmberCNAmberRemapBlend]
            type = Buffer
            stride = 32
            filename = AmberCNAmberRemapBlend.buf

            ; ---------------------------------------------



:raw-html:`<br />`

Fixing Entire Mods to a Specific Version of the Game
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The example below shows fixing entire mods to an older version of the game (before the 'Great Hash Update') after running `example.py`. If you do not specify any version, the fix will assume to fix to the latest game version available.

.. note::
    The hashes, indices and the vertex group remaps for the Blend.buf files are all fixed to the older version of the game. (The fix basically travelled in time!)

    To fix only a .ini file to a specific version of the game, go to :ref:`Fixing a .ini File to a Specific Version of the Game <apiExamples:Fixing a .ini File to a Specific Version of the Game>`

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
            hash = f35340d5
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

        # fromVersion: the version the mods were made for, version: the version to fix the mods to
        fixService = AGR.RemapServiceCLI(verbose = False, keepBackups = False, types = ["BaronBunnyCN"], version = "4.0", fromVersion = "4.0")
        fixService.fix()


.. dropdown:: Result
    :animate: fade-in-slide-down

    Below contains the new content with the fix applied for the game version 4.0

    .. dropdown:: File Structure
        :animate: fade-in-slide-down

        .. code-block::
            :emphasize-lines: 21

            AmberCN
            |
            +--> AmberCN.ini
            |
            +--> AmberCNAmberRemapBlend.buf
            |
            +--> AmberCNBlend.buf
            |
            +--> AmberCNFaceDiffuseRemapDL.dds
            |
            +--> AmberCNHeadDiffuseRemapDL.dds
            |
            +--> AmberCNHeadLightMapRemapDL.dds
            |
            +--> AmberCNHeadRemapDL.ib
            |
            +--> AmberCNPositionRemapDL.buf
            |
            +--> AmberCNTexcoordRemapDL.buf
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
            hash = f35340d5
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
            ; AmberCN remapped by Albert Gold#2696 and NK#1321. If you used it to remap your AmberCN mods pls give credit for "Albert Gold#2696" and "Nhok0169"
            ; Thank nguen#2011 SilentNightSound#7430 HazrateGolabi#1364 for support

            [TextureOverrideAmberCNHeadAmberRemapFix]
            ib = ResourceAmberCNHeadIbRemapDL
            ps-t1 = ResourceAmberCNHeadLightMapRemapDL
            hash = 9976d124
            match_first_index = 0
            ps-t0 = ResourceAmberCNHeadDiffuseRemapDL

            [TextureOverrideAmberCNBodyAmberRemapFix]
            hash = 9976d124
            match_first_index = 5670
            ib = ResourceAmberCNBodyIB
            ps-t0 = ResourceAmberCNBodyDiffuse
            ps-t1 = ResourceAmberCNBodyLightMap
            ps-t2 = ResourceAmberCNBodyMetalMap
            ps-t3 = ResourceAmberCNBodyShadowRamp

            [TextureOverrideAmberCNAmberRemapBlend]
            hash = ca5bd26e
            vb1 = ResourceAmberCNAmberRemapBlend
            handling = skip
            draw = 21916,0

            [TextureOverrideAmberCNAmberRemapPositionRemapFix]
            hash = caddc4c6
            vb0 = ResourceAmberCNPositionRemapDL

            [TextureOverrideAmberCNAmberRemapTexcoordRemapFix]
            hash = e3047676
            vb1 = ResourceAmberCNTexcoordRemapDL

            [TextureOverrideAmberCNFaceAmberRemapFix]
            hash = 1d064079
            ps-t0 = ResourceAmberCNFaceDiffuseRemapDL

            [ResourceAmberCNHeadDiffuseRemapDL]
            filename = AmberCNHeadDiffuseRemapDL.dds

            [ResourceAmberCNHeadLightMapRemapDL]
            filename = AmberCNHeadLightMapRemapDL.dds

            [ResourceAmberCNHeadIbRemapDL]
            type = Buffer
            format = DXGI_FORMAT_R32_UINT
            filename = AmberCNHeadRemapDL.ib

            [ResourceAmberCNFaceDiffuseRemapDL]
            filename = AmberCNFaceDiffuseRemapDL.dds

            [ResourceAmberCNPositionRemapDL]
            type = Buffer
            stride = 40
            filename = AmberCNPositionRemapDL.buf

            [ResourceAmberCNTexcoordRemapDL]
            type = Buffer
            stride = 12
            filename = AmberCNTexcoordRemapDL.buf

            [ResourceAmberCNAmberRemapBlend]
            type = Buffer
            stride = 32
            filename = AmberCNAmberRemapBlend.buf

            ; ---------------------------------------------


