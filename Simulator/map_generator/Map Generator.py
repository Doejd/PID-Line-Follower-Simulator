import pygame

pygame.init()

WIDTH = 828
HEIGHT = 560
LINE_WIDTH = 3

track = pygame.Surface((WIDTH, HEIGHT))
track.fill("white")

points : list[tuple[int, int]] = [

    (414, 560),  # start bottom center
    (414, 450),

    (280, 390),  # sharp turn left
    (280, 300),

    (480, 250),  # sharp turn right
    (480, 170),

    (650, 120),  # sharp right
    (650, 60),

    (500, 0)
]

pygame.draw.lines(track, "black", False, points, LINE_WIDTH)

pygame.image.save(track, "map_resources/track1.png")