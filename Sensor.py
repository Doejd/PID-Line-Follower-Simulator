from MapLoader import MapLoader


class Sensor:
    def __init__(self, x : int, y : int):
        self.x = x
        self.y = y

    def move(self, dx : int, dy : int):
        self.x += dx
        self.y += dy

    def get_color_(self, map : MapLoader):
        return map.get_pixel_color(self.x, self.y)
