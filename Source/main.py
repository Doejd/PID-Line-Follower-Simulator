import pygame, sys
from config import FPS, SCREEN_SIZE
from MapLoader import MapLoader
from Robot import Hermes

pygame.init()
screen = pygame.display.set_mode(SCREEN_SIZE)


def main() -> None:
    map_loader : MapLoader = MapLoader()
    hermes = Hermes(250, 300)
    clock = pygame.time.Clock()
    running = True
    while running:
        screen.fill("black")
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
                pygame.quit()
                sys.exit()
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_z:
                    print(hermes.get_sensors_output(map_loader))
        hermes.move(30, 40, 1/FPS)
        map_loader.draw(screen)
        hermes.draw(screen)
        pygame.display.flip()
        clock.tick(FPS)

if __name__ == "__main__":
    main()