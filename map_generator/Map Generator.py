import pygame

pygame.init()

WIDTH = 828
HEIGHT = 560
LINE_WIDTH = 3

track = pygame.Surface((WIDTH, HEIGHT))
track.fill("white")

points : list[tuple[int, int]] = [
    (621, 0),
    (621, 200),
    (300, 200),
    (300, 300),
    (621, 300),
    (621, 560),
]

pygame.draw.lines(track, "black", False, points, LINE_WIDTH)

pygame.image.save(track, "map_resources/track.png")