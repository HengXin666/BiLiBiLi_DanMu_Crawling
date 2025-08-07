class WebSocketVo:
    @staticmethod
    def log(data=None):
        return {
            "type": "log",
            "data": data,
        }

    @staticmethod
    def msg(type: str, data=None):
        return {
            "type": type,
            "data": data,
        }
