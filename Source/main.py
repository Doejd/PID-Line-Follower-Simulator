import pygame, sys
from config import FPS
from MapLoader import MapLoader
from Robot import Hermes

pygame.init()
screen = pygame.display.set_mode((900, 600)) # 828 by 560 (2*size of board in cm to pixels) with some margin


def main() -> None:
    map_loader : MapLoader = MapLoader()
    hermes = Hermes(200, 300)
    clock = pygame.time.Clock()
    running = True
    while running:
        screen.fill("black")
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                pygame.quit()
                sys.exit()
        map_loader.draw(screen)
        hermes.draw(screen)
        pygame.display.flip()
        clock.tick(FPS)

if __name__ == "__main__":
    main()