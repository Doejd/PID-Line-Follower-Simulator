import pygame
import sys
from config import FPS, SCREEN_SIZE, MARGIN
from MapLoader import MapLoader
from Robot import Robot

def main() -> None:
    pygame.init()
    screen = pygame.display.set_mode(SCREEN_SIZE)
    map_loader: MapLoader = MapLoader()
    hermes = Robot(615 + MARGIN[0], 500, starting_angle=90)
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

        map_loader.draw(screen)
        hermes.draw(screen)
        pygame.display.flip()
        clock.tick(FPS)


if __name__ == "__main__":
    main()



    