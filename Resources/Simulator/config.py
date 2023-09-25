class TEST_PATH:
    os_name = ""
    
    Number_of_Elevators = 5

    This_Elevator_Number = 1
    This_Elevator_Number_str = rf""

    Default_Timestamp = rf""

    Installation_Location_Linux = rf""
    Installation_Location_windows = rf""

    Result_Folder_Location_Linux = rf""
    Result_Folder_Location_windows = rf""

    Specific_Result_Folder_Location_Linux = rf""
    Specific_Result_Folder_Location_windows = rf""

    Configs_Folder_Location_Linux = rf""
    Configs_Folder_Location_windows = rf""

    Videos_Folder_Location_Linux = rf""
    Videos_Folder_Location_windows = rf""

    Videos_Remainder_Folder_Location_Linux = rf""
    Videos_Remainder_Folder_Location_Windows = rf"E:\ML\Elevator Git\Effective-Elevator-Energy-Calculation-for-SejongAI-Center\Videos"

    Pictures_Folder_Location_Linux = rf""
    Pictures_Folder_Location_windows = rf""

    Logs_Folder_Location_Linux = rf""
    Logs_Folder_Location_windows = rf""

    sample_image_linux = rf""
    sample_image_windows = rf"E:\ML\Elevator Git\Elevator_Results\Pictures\No5\20230912_142324"

    #sample_image_windows = rf"E:\ML\Elevator Git\Elevator_Results\Pictures\No5\Test"
    #sample_image_windows = rf"E:\ML\Elevator Git\Elevator_Results"

    yaml_linux = rf""
    yaml_windows = rf"E:\ML\Elevator Git\Elevator_Results\Configs\data.yaml"

    label_linux = rf""
    label_windows = rf""

    image_folder_linux = rf""
    image_folder_windows = rf""

    button_list_linux = rf""
    button_list_windows = rf"E:\ML\Elevator Git\Elevator_Results\test.txt"

class TEST_COLOR:
    #lowest_RGB = [130, 185, 60]
    #highest_RGB = [225, 245, 120]
    
    lowest_RGB = [0, 0, 0]
    highest_RGB = [225, 245, 220]

class TEST_VARIABLES:
    min_circle_area = 1.0
    min_total_dot_when_green = 20
    min_confidence_rate = 0.01
    class_id = ["Open", "Close", "B5", "B4", "B3", "B2", "B1", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "Panel"]

    #Elevator_Alt = [-45, -41.5, -38, -34.5, -31, -28, -22, -18, -15, -12, -9, -6, -3, 0, 3, 6, 9]
    Elevator_Alt = [-55, -51.5, -48, -44.5, -41, -38, -32, -28, -25, -22, -19, -16, -13, 10, 7, 4, 1]
