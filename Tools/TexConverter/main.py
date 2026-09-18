import argparse
import sys

from TexConverter.TexConverter import TexConverter


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description = "Converts .dds textures into an ordinary image format (.png by default), so they can actually be viewed",
        epilog = "Examples:\n"
                 "  python3 main.py \"SomeMod/AmberBodyDiffuse.dds\" \"C:/scratch/amber.png\"\n"
                 "  python3 main.py \"SomeMod/AmberBodyDiffuse.dds\" \"C:/scratch\"\n"
                 "  python3 main.py \"SomeMod\" \"C:/scratch/textures\"",
        formatter_class = argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("src", type = str, help = "the .dds file, or a folder of .dds files, to convert")
    parser.add_argument("dest", type = str, help = "where to write the result -- an output file, or a folder to put the output in (required: this tool never writes next to the source, to avoid littering mod/data folders)")
    parser.add_argument("-f", "--format", type = str, default = "png", help = "the image format to convert to (default: png)")
    parser.add_argument("-a", "--alpha", type = str, default = TexConverter.AlphaDrop, choices = TexConverter.AlphaModes,
                        help = "how to treat the alpha channel: 'drop' (default) forces every pixel opaque so the RGB artwork is always visible -- these textures often use alpha as a mask rather than transparency, and honouring it can render a texture blank; 'keep' writes all 4 channels faithfully; 'only' writes the alpha channel by itself, as greyscale")
    parser.add_argument("-s", "--silent", action = "store_true", help = "don't print each conversion")

    args = parser.parse_args()

    try:
        written = TexConverter(args.src, args.dest, format = args.format, alpha = args.alpha).convert(silent = args.silent)
    except (FileNotFoundError, ImportError) as e:
        print(f"error: {e}", file = sys.stderr)
        sys.exit(1)

    sys.exit(0 if (written) else 1)
