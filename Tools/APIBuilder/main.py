from APIBuilder import APIBuilder, CommandBuilder
from APIBuilder.constants.BuildEnv import BuildEnv


if __name__ == '__main__':
    command = CommandBuilder()
    args = command.parse()

    # the 'core' environment builds only the C++ SDK: it leaves the Python package's compiled modules
    #   alone (it does not replace them), and has no Python stubs to generate docs from
    isCore = args.env == BuildEnv.Core

    apiBuilder = APIBuilder(args.env, installPath = args.installFolder, cleanBuild = args.buildRemove, cleanPreInstall = args.preinstallRemove,
                            cleanInstall = not args.installKeep and not isCore, makeBuild = not args.skipBuild, addDocs = args.addDocs and not isCore,
                            addCredits = args.addCredits,
                            makePreBuild = args.makePreBuild, makePreInstall = args.makePreInstall,
                            preBuildSuffix = args.prebuildSuffix, buildSuffix = args.buildSuffix, preInstallSuffix = args.preinstallSuffix,
                            buildLocation = args.buildLocation)
    apiBuilder()
