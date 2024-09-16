# Error: The base exception used
class Error(Exception):
    def __init__(self, message: str):
        super().__init__(f"ERROR: {message}")