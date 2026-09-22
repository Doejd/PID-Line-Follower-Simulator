import pygame

pygame.init()

WIDTH = 828
HEIGHT = 560
LINE_WIDTH = 3

track = pygame.Surface((WIDTH, HEIGHT))
track.fill("white")

points : list[tuple[int, int]] = [
    (621, 0),
    (200, 300),
    (400, 250),
    (600, 300),
    (800, 500),
    (621, 840),
]

pygame.draw.lines(track, "black", False, points, LINE_WIDTH)

pygame.image.save(track, "../Map Resources/track.png")