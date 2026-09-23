from typing import Optional
from pathlib import Path
import pygame
from config import MARGIN

class MapLoader:
    def __init__(self, directory : str = "../Map Resources", filename: Optional[str] = None):
        self.directory = Path(directory)
        self.map_image = self.open_image_map(filename)

    def open_image_map(self, filename : Optional[str] = None) -> pygame.Surface:
        if not self.directory.is_dir():
            raise FileNotFoundError("Directory not found")

        if filename:
            target_file = self.directory / filename
            if not target_file.is_file():
                raise FileNotFoundError(f"File not found {target_file}")
            return pygame.image.load(str(target_file))

        png_files = sorted(self.directory.glob("*.png"))
        if not png_files:
            raise FileNotFoundError("No .png files found in directory")
        return pygame.image.load(str(png_files[0]))

    def get_pixel_color(self, x : int, y : int):
        x_ = int(x) - MARGIN[0] # convert from global coordinates to local coordinates
        y_ = int(y) - MARGIN[1] # convert from global coordinates to local coordinates
        if not (0 <= x_ < self.map_image.get_width() and 0 <= y_ < self.map_image.get_height()):
            raise IndexError(f"Coordinates ({x}, {y}) are out of bounds")
        return self.map_image.get_at((x_, y_))

    def draw(self, surface):
        surface.blit(self.map_image, MARGIN)