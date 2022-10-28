import glob
from plot_string_count import main

# Started at 3:57 pm


if __name__ == "__main__":
    folder_names = list(glob.glob(f"../build/data/100_trials_ca_N*"))
    check_list = []
    for folder_name in folder_names:
        identifier = folder_name[-5:]
        args = [folder_name, "200", "2800"]
        bad_bit_flag = main(args)
        if bad_bit_flag:
            check_list.append(identifier)

    if len(check_list) == 0:
        print("There were no bad fits!")
    else:
        print("The following produce bad fits:")
        for identifier in check_list:
            print(identifier)
