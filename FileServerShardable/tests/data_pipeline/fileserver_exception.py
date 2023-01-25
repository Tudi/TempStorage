class FileServerException(Exception):
    key: str
    message: str
    log: bool
    deadpool: bool

    not_found: bool
    save_failed: bool

    def __init__(
            self,
            key: str,
            message: str,
            log: bool = True,
            deadpool: bool = False,
            not_found: bool = False,
            save_failed: bool = False
    ):
        super().__init__(f"{key}: {message}")
        self.key = key
        self.message = message
        self.log = log
        self.deadpool = deadpool
        self.not_found = not_found
        self.save_failed = save_failed

    def __repr__(self):
        return f"{self.key}, {self.message}: log {self.log}, deadpool {self.deadpool}," \
            f" not_found {self.not_found}, save_failed: {self.save_failed}."
