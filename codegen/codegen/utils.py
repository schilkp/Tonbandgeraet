def pad_to_length(s: str, l: int, c: str) -> str:
    return s + (c * (l - len(s)))
