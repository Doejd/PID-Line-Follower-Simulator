import pygame
import sys

from Source.config import MARGIN
from config import FPS, SCREEN_SIZE
from MapLoader import MapLoader
from Robot import Robot

def CalcError(hermes : Robot, map_loader : MapLoader) -> int:
    weights  = [8-x for x in range(17) if x-8 != 0]
    error = 0
    for i, sensor in enumerate(hermes.sensors):
        color = sensor.get_color(map_loader)
        isBlack : bool = (color == (0, 0, 0, 255))
        error += isBlack * weights[i]
    return error

def main() -> None:
    pygame.init()
    screen = pygame.display.set_mode(SCREEN_SIZE)
    map_loader: MapLoader = MapLoader()
    hermes = Robot(615+MARGIN[0], 550-MARGIN[1], starting_angle=90)
    clock = pygame.time.Clock()
    running = True

    prevError = 0
    errorSum = 0

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

        clock.tick(FPS)

        map_loader.draw(screen)
        hermes.draw(screen)
        pygame.display.flip()


        error = CalcError(hermes, map_loader)
        speed = 60
        Kp = 20
        Kd = 3
        Ki = 0.1

        errorDiff = Kp - prevError
        prevError = Kp
        errorSum += error
        correction = error * Kp + errorDiff * Kd + errorSum * Ki

        hermes.move(correction , speed, 1/FPS)


if __name__ == "__main__":
    main()



    