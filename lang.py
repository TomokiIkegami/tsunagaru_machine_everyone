from googletrans import Translator
import langid


class Car:
    def __init__(self):
        self.speed = 0
        self.direction = "straight"

    def accelerate(self, amount):
        self.speed += amount
        print(f"Accelerating. Current speed: {self.speed} km/h")

    def brake(self, amount):
        if self.speed - amount >= 0:
            self.speed -= amount
            print(f"Braking. Current speed: {self.speed} km/h")
        else:
            print("Car is already stopped.")

    def turn_left(self):
        self.direction = "left"
        print("Turning left.")

    def turn_right(self):
        self.direction = "right"
        print("Turning right.")

    def straighten(self):
        self.direction = "straight"
        print("Straightening.")

def control_car(translated_command, car):
    if translated_command == 'exit':
        print("Exiting the control system.")
        return False

    elif translated_command == 'accelerate':
        try:
            amount = float(input("Enter acceleration amount: "))
            car.accelerate(amount)
        except ValueError:
            print("Invalid input. Please enter a numeric value for acceleration.")

    elif translated_command == 'brake':
        try:
            amount = float(input("Enter braking amount: "))
            car.brake(amount)
        except ValueError:
            print("Invalid input. Please enter a numeric value for braking.")

    elif translated_command == 'left':
        car.turn_left()

    elif translated_command == 'right':
        car.turn_right()

    elif translated_command == 'straight':
        car.straighten()

    else:
        print("Invalid command. Please enter a valid command.")

    return True

def translate_to_english(text, source_language):
    try:
        translator = Translator()
        translated_text = translator.translate(text, src=source_language, dest='en').text
        return translated_text
    except Exception as e:
        print(f"Translation error: {e}")
        return None

def change_language_prompt():
    new_language = input("Enter the new language code: ")
    print(f"Source language changed to {new_language}")
    return new_language

if __name__ == "__main__":
    car = Car()

    source_language = input("Enter the initial language code (zh for Chinese, ja for Japanese, id for Indonesian): ")
    print(f"Source language set to {source_language}")

    while True:
        user_input = input("Enter a few words (type 'exit' to end): ")

        if user_input.lower() == 'exit':
            print("Exiting the translation and control.")
            break

        try:
            if user_input.lower() == 'ganti bahasa' and source_language == 'id':
                source_language = change_language_prompt()
                continue

            translated_command = translate_to_english(user_input, source_language)
            print(f"Translated to English: {translated_command}")
            if not control_car(translated_command, car):
                break
        except Exception as e:
            print(f"Translation error: {e}")
            print("Unable to translate. Please try again.")
