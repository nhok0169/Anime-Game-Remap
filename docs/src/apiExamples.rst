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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile("CuteLittleRaiden.ini", modTypes = FRB.ModTypes.getAll())
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


        ; --------------- Raiden Boss Fix ---------------
        ; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
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


        ; -----------------------------------------------


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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile(txt = shortWackyRaidenIniTxt, modTypes = FRB.ModTypes.getAll())
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



        ; --------------- Raiden Boss Fix ---------------
        ; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
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


        ; -----------------------------------------------

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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile(txt = shortWackyRaidenIniTxt, modTypes = FRB.ModTypes.getAll())
        iniFile.parse()
        fixCode = iniFile.getFixStr()

        print(fixCode)


.. dropdown:: Result
    :animate: fade-in-slide-down

    .. code-block:: ini
        :linenos:

        ; --------------- Raiden Boss Fix ---------------
        ; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
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


        ; -----------------------------------------------


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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile("PartiallyFixedRaiden.ini", modTypes = FRB.ModTypes.getAll())
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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile(txt = showWackyRaidenIniTxtWithFix, modTypes = FRB.ModTypes.getAll())
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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile("PartiallyFixedRaiden.ini", modTypes = FRB.ModTypes.getAll())
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




        ; --------------- Raiden Boss Fix ---------------
        ; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
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


        ; -----------------------------------------------
        

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

        import FixRaidenBoss2 as FRB

        iniFile = FRB.IniFile(txt = showWackyRaidenIniTxtWithFix, modTypes = FRB.ModTypes.getAll())
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




        ; --------------- Raiden Boss Fix ---------------
        ; Raiden Boss fixed by NK#1321 if you used it for fix your Raiden mods pls give credit for "Nhok0169"
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


        ; -----------------------------------------------


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

        import FixRaidenBoss2 as FRB

        FRB.Mod.blendCorrection("../LittleEiBlend.buf", FRB.ModTypes.Raiden.value, "PuppetEiGotRemapped.buf")


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

        import FixRaidenBoss2 as FRB

        fixedBytes = FRB.Mod.blendCorrection(inputBytes, FRB.ModTypes.Raiden.value)
        print(fixedBytes)


.. dropdown:: Result
    :animate: fade-in-slide-down

    The bytes that fixes ``LittleEiBlend.buf``



:raw-html:`<br />`
:raw-html:`<br />`

Fixing Entire Mods
------------------

The below examples simulate executing the entire script, but through the API


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

        import FixRaidenBoss2 as FRB

        fixService = FRB.BossFixService(path = "../../", verbose = False, keepBackups = False)
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

        import FixRaidenBoss2 as FRB

        fixService =FRB.RaidenBossFixService(path = "../../", verbose = True, keepBackups = False, undoOnly = True)
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