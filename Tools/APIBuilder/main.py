from APIBuilder import APIBuilder, CommandBuilder


if __name__ == '__main__':
    command = CommandBuilder()
    args = command.parse()

    apiBuilder = APIBuilder(args.env, installPath = args.installFolder, cleanBuild = not args.buildKeep,
                            cleanInstall = not args.installKeep, makeBuild = not args.skipBuild, addDocs = args.addDocs)
    apiBuilder()
