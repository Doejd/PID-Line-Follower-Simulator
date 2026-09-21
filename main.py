import pygame, sys
from config import FPS
from MapLoader import MapLoader

pygame.init()
screen = pygame.display.set_mode((1300, 900)) # 1242 by 840 (3*size of board in cm to pixels) with some margin


def main() -> None:
    map_loader : MapLoader = MapLoader()
    clock = pygame.time.Clock()
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                pygame.quit()
                sys.exit()
        map_loader.draw(screen)
        pygame.display.flip()
        clock.tick(FPS)

if __name__ == "__main__":
    main()