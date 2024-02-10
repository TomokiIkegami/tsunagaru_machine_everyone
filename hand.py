import cv2
from cvzone.HandTrackingModule import HandDetector

class HandDirectionDetector:
    def __init__(self):
        self.detector = HandDetector(detectionCon=0.8, maxHands=1)

    def determine_direction(self, finger_count):
        if finger_count == 1:
            return 'forward'
        elif finger_count == 2:
            return 'backward'
        elif finger_count == 3:
            return 'right_and_backward'
        elif finger_count == 4:
            return 'left_and_backward'
        elif finger_count == 5:
            return 'right_and_forward'
        elif finger_count == 0:
            return 'left_and_forward'
        else:
            return 'stop'

    def detect_hand_direction(self, frame):
        hands, img = self.detector.findHands(frame)
        
        if hands:
            lmList = hands[0]

            finger_count = self.detector.fingersUp(lmList)
            cv2.putText(frame, f'Finger count: {sum(finger_count)}', (10, 30), cv2.FONT_HERSHEY_COMPLEX, 1, (255, 255, 255), 1,
                        cv2.LINE_AA)

            direction = self.determine_direction(sum(finger_count))

            cv2.putText(frame, f'Direction: {direction}', (10, 60), cv2.FONT_HERSHEY_COMPLEX, 1, (0, 255, 0), 2,
                        cv2.LINE_AA)

            return direction

        return None