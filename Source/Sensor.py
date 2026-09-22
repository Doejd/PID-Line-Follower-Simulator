from MapLoader import MapLoader
import pygame

class Sensor:
    def __init__(self, x : float, y : float):
        self.x = x
        self.y = y

    def move(self, dx : float, dy : float):
        self.x += dx
        self.y += dy

    def get_color(self, map : MapLoader):
        return map.get_pixel_color(round(self.x), round(self.y))

    def draw(self, surface):
        pygame.draw.circle(surface, (0, 255, 0), (self.x, self.y), 5)
