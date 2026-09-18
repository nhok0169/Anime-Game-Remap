import os

# Unlike the other tools in this folder, these are absolute (derived from this file's own location)
# rather than relative to the current working directory -- this tool is meant to be run by path from
# anywhere (including by an AI agent that never cd's into this folder first), so a CWD-relative path
# would break the import of the API below.
ToolPath = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RepoPath = os.path.dirname(os.path.dirname(ToolPath))
ProjectPath = os.path.join(RepoPath, r"Anime Game Remap (for all users)")
APIPath = os.path.join(ProjectPath, "api")
APISrcPath = os.path.join(APIPath, "src", "py")
