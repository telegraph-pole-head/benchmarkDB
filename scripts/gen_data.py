import csv
import pathlib
import random
import string
import time

from tqdm import tqdm


def generate_name(existing_names):
    while True:
        first_name = "".join(
            random.choices(string.ascii_letters, k=random.randint(3, 8))
        )
        last_name = "".join(
            random.choices(string.ascii_letters, k=random.randint(3, 8))
        )
        name = f"{first_name}_{last_name}"
        if name not in existing_names:
            existing_names.add(name)
            return name


def generate_student_id(existing_ids):
    while True:
        student_id = "".join(random.choices(string.digits, k=10))
        if student_id not in existing_ids:
            existing_ids.add(student_id)
            return student_id


def generate_class():
    return random.randint(2010, 2020)


def generate_total_credit():
    mean = 110
    std_dev = 15  # Standard deviation
    total_credit = int(random.normalvariate(mean, std_dev))
    return max(0, total_credit)  # Ensure total credit is not negative


def generate_data(n):
    existing_names = set()
    existing_ids = set()
    data = [
        [
            generate_name(existing_names),
            generate_student_id(existing_ids),
            generate_class(),
            generate_total_credit(),
        ]
        for _ in tqdm(range(n), desc="Generating data")
    ]
    return data


def main():
    n = int(input("Enter the number of rows to generate: "))

    # record the time it takes to generate the data
    start = time.time()
    data = generate_data(n)
    end = time.time()
    print(f"Time taken to generate data: {end - start} seconds")
    csv_path = pathlib.Path(__file__).parent.parent.joinpath("data/data.csv")
    with open(csv_path, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["KEY", "studentID", "class", "totalCredit"])
        writer.writerows(data)
    print(f"{n} rows of data written to {csv_path}")


if __name__ == "__main__":
    main()
