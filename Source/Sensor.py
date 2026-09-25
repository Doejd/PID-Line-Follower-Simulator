from MapLoader import MapLoader
from config import GREEN
import pygame

class Sensor:
    def __init__(self, x: float, y: float):
        self.x = x
        self.y = y

    def get_color(self, track_map: MapLoader):
        return track_map.get_pixel_color(round(self.x), round(self.y))

    def draw(self, surface):
        pygame.draw.circle(surface, GREEN, (self.x, self.y), 5)
