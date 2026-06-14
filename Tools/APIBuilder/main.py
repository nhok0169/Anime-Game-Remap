from APIBuilder import APIBuilder, CommandBuilder


if __name__ == '__main__':
    command = CommandBuilder()
    args = command.parse()

    apiBuilder = APIBuilder(args.env, installPath = args.installFolder, cleanBuild = args.buildRemove, cleanPreInstall = args.preinstallRemove,
                            cleanInstall = not args.installKeep, makeBuild = not args.skipBuild, addDocs = args.addDocs,
                            makePreBuild = args.makePreBuild, makePreInstall = args.makePreInstall,
                            preBuildSuffix = args.prebuildSuffix, buildSuffix = args.buildSuffix, preInstallSuffix = args.preinstallSuffix)
    apiBuilder()
