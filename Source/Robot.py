import math
from pathlib import Path
from typing import Optional
import pygame
from Sensor import Sensor
from MapLoader import MapLoader
from config import ROBOT_WIDTH, WHEEL_BASE, ROBOT_HEIGHT
from config import RED, SENSOR_RADIUS

class Robot:
    def __init__(self, x: float, y: float, number_of_sensors: int = 16, image_path: Optional[str] = None):
        self.pos = pygame.Vector2(x, y)
        self.angle = 0
        self.acceleration = 200
        self.turn_speed = 120
        self.speed = 0
        self.number_of_sensors = number_of_sensors
        self.sensors: list[Sensor] = self.set_sensors()
        self.image: pygame.Surface = self.load_robot_image(image_path)

    @staticmethod
    def load_robot_image(image_path: Optional[str] = None) -> pygame.Surface:
        """
        Loads and scales the robot sprite to fit robot dimensions.
        """
        paths_to_try = []
        if image_path:
            paths_to_try.append(Path(image_path))
        paths_to_try.extend([
            Path("../robot_resources/robot.png"),
            Path("robot_resources/robot.png"),
            Path(__file__).resolve().parent.parent / "robot_resources" / "robot.png",
        ])
        for path in paths_to_try:
            if path.is_file():
                try:
                    img = pygame.image.load(str(path)).convert_alpha()
                except pygame.error:
                    img = pygame.image.load(str(path))
                return pygame.transform.smoothscale(img, (ROBOT_WIDTH, ROBOT_HEIGHT))

        # Fallback surface if image is not found
        surface = pygame.Surface((ROBOT_WIDTH, ROBOT_HEIGHT), pygame.SRCALPHA)
        pygame.draw.rect(surface, RED, (0, 0, ROBOT_WIDTH, ROBOT_HEIGHT))
        return surface

    def set_sensors(self) -> list[Sensor]:
        """
        This is not supposed to be used publicly.
        :return: A list of sensors.
        """
        sensors: list[Sensor] = []
        angle_step = 180 / (self.number_of_sensors - 1)
        center = (self.pos.x + ROBOT_WIDTH / 2, self.pos.y + ROBOT_HEIGHT / 2)
        for i in range(self.number_of_sensors):
            angle_deg = self.angle + 90 - i * angle_step
            angle_rad = math.radians(angle_deg)

            x = SENSOR_RADIUS * math.cos(angle_rad)
            y = SENSOR_RADIUS * math.sin(angle_rad)
            sensors.append(Sensor(center[0] + x, center[1] - y))
        return sensors

    def move(self, steering_angle: float, speed: float, delta: float) -> None:
        """
            Moves the robot based on the given steering angle, speed, and time delta.
        :param steering_angle: The steering angle of the robot.
        :param speed: The speed of the robot.
        :param delta: The time delta for the movement.
        :return: None
        """
        angle_rad = math.radians(self.angle)
        steering_angle_rad = math.radians(steering_angle)

        distance = speed * delta
        self.pos.x += distance * math.cos(angle_rad)
        self.pos.y -= distance * math.sin(angle_rad)

        angular_velocity = speed / WHEEL_BASE * math.tan(steering_angle_rad)
        self.angle += math.degrees(angular_velocity * delta)

        self.sensors = self.set_sensors()


    def get_sensors_output(self, track_map: MapLoader) -> list[pygame.Color]:
        """
            Returns a list of colors detected by the robot's sensors.
        Args
        :param track_map: The map loader object.
        :return: A list of colors detected by the robot's sensors.
        """
        for sensor in self.sensors:
            print(f"Sensor at {sensor.x}, {sensor.y} sees {sensor.get_color(track_map)}") # !!! Uncomment for debug info
        return [sensor.get_color(track_map) for sensor in self.sensors]

    def draw(self, surface: pygame.Surface, draw_sensors: bool = True) -> None:
        """
            Draws the robot body at the correct rotation.
        Args
        :param surface: The surface to draw the robot on.
        :param draw_sensors: Whether to draw the robot's sensors.
        :return: None
        """
        center = (self.pos.x + ROBOT_WIDTH / 2, self.pos.y + ROBOT_HEIGHT / 2)
        rotated_surface = pygame.transform.rotate(self.image, self.angle)
        rect = rotated_surface.get_rect(center=center)
        surface.blit(rotated_surface, rect.topleft)

        if draw_sensors:
            for sensor in self.sensors:
                sensor.draw(surface)