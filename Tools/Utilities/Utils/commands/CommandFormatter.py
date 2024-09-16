import argparse


# CommandFormatter: Text formatting for the help page of the command 
class CommandFormatter(argparse.MetavarTypeHelpFormatter, argparse.RawTextHelpFormatter):
    pass