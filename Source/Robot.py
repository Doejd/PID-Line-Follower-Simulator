import math

import pygame

from Sensor import Sensor
from MapLoader import MapLoader

class Hermes:
    def __init__(self, x : float, y : float, number_of_sensors : int = 16):
        self.x = x
        self.y = y
        self.width = 25
        self.height = 25
        self.center = (self.x + self.width / 2, self.y + self.height / 2)
        self.sensor_radius = 9.57
        self.number_of_sensors = number_of_sensors
        self.sensors : list[Sensor] = self.set_sensors()

    def set_sensors(self):
        sensors : list[Sensor] = []
        angle_step = 180 / self.number_of_sensors - 1
        for i in range(self.number_of_sensors):
            angle_deg = i * angle_step
            angle_rad = math.radians(angle_deg)

            x = self.sensor_radius * math.cos(angle_rad)
            y = self.sensor_radius * math.sin(angle_rad)
            sensors.append(Sensor(self.center[0] - x, self.center[1] - y))
        return sensors

    def move(self, dx : float, dy : float):
        self.x += dx
        self.y += dy
        for sensor in self.sensors: sensor.move(dx, dy)

    def get_sensors_output(self, map : MapLoader):
        #print(f"Hermes is at {self.center[0]}, {self.center[1]}") # !!! Uncomment for debug info
        #for sensor in self.sensors: print(f"Sensor at {sensor.x}, {sensor.y} sees {sensor.get_color(map)}") # !!! Uncomment for debug info
        return [sensor.get_color(map) for sensor in self.sensors] # comment when debugging

    def draw(self, surface : pygame.Surface):
        # for sensor in self.sensors: sensor.draw(surface) !!! Uncomment for debug info
        pygame.draw.rect(surface, (255, 0, 0), (self.x, self.y, self.width, self.height))