import math
import pygame
from Sensor import Sensor
from MapLoader import MapLoader

class Hermes:
    def __init__(self, x : float, y : float, number_of_sensors : int = 16):
        self.pos = pygame.Vector2(x, y)
        self.width = 25
        self.height = 25
        self.sensor_radius = 9.57
        self.angle = 45
        self.acceleration = 200
        self.turn_speed = 120
        self.speed = 0
        self.number_of_sensors = number_of_sensors
        self.sensors : list[Sensor] = self.set_sensors()

    def set_sensors(self):
        sensors : list[Sensor] = []
        angle_step = 180 / self.number_of_sensors - 1
        center = (self.pos.x + self.width / 2, self.pos.y + self.height / 2)
        for i in range(self.number_of_sensors):
            angle_deg = i * angle_step
            angle_rad = math.radians(angle_deg)

            x = self.sensor_radius * math.cos(angle_rad)
            y = self.sensor_radius * math.sin(angle_rad)
            sensors.append(Sensor(center[0] - x, center[1] - y))
        return sensors

    def move(self, steering_angle : float, speed : float, delta : float):
        angle_rad = math.radians(self.angle)
        steering_angle_rad = math.radians(steering_angle)

        distance = speed * delta
        self.pos.x += distance * math.cos(angle_rad)
        self.pos.y -= distance * math.sin(angle_rad)

        angular_velocity = speed / (self.width - 5) * math.tan(steering_angle_rad)
        self.angle += math.degrees(angular_velocity * delta)

        angle = math.radians(self.angle)
        for sensor in self.sensors:
            rotated_x = (
                sensor.x * math.cos(angle)
                - sensor.y * math.sin(angle)
            )

            rotated_y = (
                sensor.x * math.sin(angle)
                + sensor.y * math.cos(angle)
            )

            sensor.x = rotated_x
            sensor.y = rotated_y


    def get_sensors_output(self, map : MapLoader):
        #print(f"Hermes is at {self.center[0]}, {self.center[1]}") # !!! Uncomment for debug info
        #for sensor in self.sensors: print(f"Sensor at {sensor.x}, {sensor.y} sees {sensor.get_color(map)}") # !!! Uncomment for debug info
        return [sensor.get_color(map) for sensor in self.sensors]

    def draw(self, surface : pygame.Surface):
        # for sensor in self.sensors: sensor.draw(surface) # !!! Uncomment for debug info
        robot_surface : pygame.Surface = pygame.Surface((self.width, self.height), pygame.SRCALPHA)
        pygame.draw.rect(robot_surface, (255, 0, 0), (0, 0, self.width, self.height))
        robot_surface = pygame.transform.rotate(robot_surface, self.angle)
        surface.blit(robot_surface, self.pos)