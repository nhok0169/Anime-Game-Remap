from ..softwareStats.SoftwareMetadata import SoftwareMetadata
from ..softwareStats.BuildMetadata import BuildMetadata
from ..softwareStats.SoftwareContributor import SoftwareContributor

Title = "Anime Game Remap"
ShortTitle = "AG Remap"
APIVersion = "4.4.0"

Authors = {
    "Albert": SoftwareContributor("Albert Gold", discName = "albertgold", oldDisName = "Albert Gold#2696"),
    "Nhok": SoftwareContributor("NK", discName = "nhok0169", oldDisName = "NK#1321")
}

AllAuthors = list(Authors.values())

ScriptStats = SoftwareMetadata(version = APIVersion, title = f"{Title} Script", shortTitle = f"{ShortTitle} Script", authors = AllAuthors)
APIStats = SoftwareMetadata(name = "FixRaidenBoss2", title = Title, shortTitle = ShortTitle, version = APIVersion, authors = AllAuthors)
APIMirrorStats = SoftwareMetadata(name = "AnimeGameRemap", title = Title, shortTitle = ShortTitle, version = APIVersion, authors = AllAuthors)
ScriptBuilderStats = SoftwareMetadata(name = "ScriptBuilder", title = "ScriptBuilder", version = "1.0.0", authors = [Authors["Albert"]])
APIMirrorBuilderStats = SoftwareMetadata(name = "APIMirrorBuilder", title = "APIMirrorBuilder", shortTitle = "MirrorBuilder", version = "1.0.0", authors = [Authors["Albert"]])
UtilityStats = SoftwareMetadata(name = "AGRemapUtils", title = "Anime Game Remap Utilities", version = "1.0.4", authors = [Authors["Albert"]])

ScriptBuildStats = BuildMetadata.fromSoftwareMetadata(ScriptStats)
ScriptBuilderBuildStats = BuildMetadata.fromSoftwareMetadata(ScriptBuilderStats)
APIMirrorBuildStats = BuildMetadata.fromSoftwareMetadata(APIMirrorStats)
APIMirrorBuilderBuildStats = BuildMetadata.fromSoftwareMetadata(APIMirrorBuilderStats)