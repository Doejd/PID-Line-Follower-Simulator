import pygame
import sys

from config import MARGIN
from config import FPS, SCREEN_SIZE
from MapLoader import MapLoader
from Robot import Robot

black = (0, 0, 0, 255)
weights = [7.5 - i for i in range(16)]

def calc_error(hermes : Robot, map_loader : MapLoader) -> int:
    sensor_colors = hermes.get_sensors_output(map_loader)
    return sum(weights[i] * (sensor_colors[i] == black) for i in range(16))

def on_the_line(hermes : Robot, map_loader : MapLoader) -> bool:
    return any(color == black for color in hermes.get_sensors_output(map_loader))

def main() -> None:
    pygame.init()
    screen = pygame.display.set_mode(SCREEN_SIZE)
    map_loader: MapLoader = MapLoader()
    hermes = Robot(615+MARGIN[0], 550-MARGIN[1], starting_angle=89)
    clock = pygame.time.Clock()
    running = True

    prev_error = 0
    error_sum = 0

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

        speed = 120
        Kp = 20
        Kd = 22
        Ki = 0.05

        
        error = calc_error(hermes, map_loader)
        if(not on_the_line(hermes, map_loader)): 
            if prev_error > 0: error = 8
            elif prev_error < 0: error = -8
            else: error = 0

            print("off the line ", error_sum)

        print(error)
        error_diff = error - prev_error
        error_sum += error
        error_sum = max(-100, min(error_sum, 100))
            

        correction = (error * Kp) + (error_diff * Kd) + (error_sum * Ki)
        prev_error = error

        hermes.move(correction , speed, 1/FPS)


if __name__ == "__main__":
    main()



    