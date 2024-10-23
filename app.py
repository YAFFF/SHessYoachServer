import subprocess
import os
import http.client
import time
from flask import Flask, render_template, jsonify
from threading import Thread
from werkzeug.serving import run_simple

app = Flask(__name__)
server_process = None
server_status = "Неактивен"
output_file_path = "server_output.txt"
crash_file_path = "server_crash.txt"
build_output_file = "output.txt"  # Output file created by nohup
project_path = os.getcwd()
build_timeout_seconds = 60  # Максимальное время на сборку

# Создаем файл 'output.txt' при запуске
with open(output_file_path, 'w') as output_file:
    output_file.write("")

def check_build_success():
    # Проверяем последние две строки файла build_output_file
    if os.path.exists(build_output_file):
        with open(build_output_file, 'r') as file:
            lines = file.readlines()
            if len(lines) >= 2:
                last_line = lines[-1].strip()
                second_last_line = lines[-2].strip()
                if "[100%] Built target SHessYoachServer" in last_line or "[100%] Built target SHessYoachServer" in second_last_line:
                    return True
    return False


def build_cpp_server():
    # Запускаем процесс сборки с таймаутом
    try:
        # Выполняем cmake для настройки проекта
        cmake_process = subprocess.Popen(
            ["cmake", "."], cwd="cmake-build-files", stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        cmake_process.communicate()  # Ожидаем завершения cmake

        # Проверяем успешность выполнения cmake
        if cmake_process.returncode != 0:
            print("CMake configuration failed.")
            return False

        # Выполняем make для сборки проекта
        build_process = subprocess.Popen(
            ["make"], cwd="cmake-build-files", stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )

        # Ждем завершения или прерываем через timeout
        start_time = time.time()
        while build_process.poll() is None:
            time_elapsed = time.time() - start_time
            if time_elapsed > build_timeout_seconds:
                build_process.terminate()
                print("Build process timed out after 60 seconds.")
                return False
            time.sleep(1)

        # Проверка успешности сборки по логам
        return check_build_success()

    except Exception as e:
        print(f"Error during build: {e}")
        return False

def start_server():
    global server_status, server_process
    if server_status == "Неактивен":
        # Сборка C++ сервера и проверка на успех
        if build_cpp_server():
            server_process = subprocess.Popen(
                ["./SHessYoachServer", "0.0.0.0", "8080"],
                cwd="cmake-build-files",
                stdout=open(output_file_path, 'a'),
                stderr=subprocess.STDOUT,
            )
            server_status = "Активен"
            monitor_server()  # Запуск мониторинга сервера
        else:
            print("Build failed or did not complete successfully.")

def stop_server():
    global server_status, server_process
    if server_status == "Активен" and server_process is not None:
        server_process.terminate()
        server_status = "Неактивен"

def has_changes():
    # Проверяем, есть ли изменения после git pull
    subprocess.run(['git', 'fetch'], check=True)
    local_branch = subprocess.run(['git', 'rev-parse', 'HEAD'], stdout=subprocess.PIPE, text=True).stdout.strip()
    remote_branch = subprocess.run(['git', 'rev-parse', 'FETCH_HEAD'], stdout=subprocess.PIPE, text=True).stdout.strip()
    return local_branch != remote_branch

def pull_and_restart():
    # Проверяем изменения
    if has_changes():
        print("Changes detected. Restarting server...")
        # Выполняем git pull
        stop_server()

        # Откатываем все изменения (включая проиндексированные файлы)
        subprocess.run(['git', 'reset', '--hard'], check=True)
        subprocess.run(['git', 'clean', '-fd'], check=True)
        subprocess.run(['git', 'pull', '--force'], check=True)

        start_server()

def monitor_server():
    global server_process
    while server_process is not None and server_process.poll() is None:
        # Сервер активен, проверка каждые 5 секунд
        time.sleep(5)

    # Если сервер завершился, обрабатываем экстренное завершение
    if server_process is not None and server_process.poll() is not None:
        handle_server_crash()

def handle_server_crash():
    global server_status, server_process
    print("Server crashed! Renaming output file and restarting...")

    # Переименовываем 'server_output.txt' в 'server_crash.txt'
    if os.path.exists(output_file_path):
        os.rename(output_file_path, crash_file_path)

    # Перезапускаем сервер
    stop_server()
    start_server()

def run_flask():
    run_simple("0.0.0.0", 8088, app, use_reloader=False)

@app.route('/')
def index():
    output_content = ""
    with open(output_file_path, 'r') as output_file:
        output_content += output_file.read()
    print(output_content)
    return render_template('index.html', server_status=server_status, output_content=output_content)

@app.route('/get_logs')
def get_logs():
    with open(output_file_path, 'r') as output_file:
        content = output_file.read()
        lines = content.split('\n')  # Split the content into lines
        last_40_lines = lines[-40:]
    return '\n'.join(last_40_lines)

@app.route('/turn_on')
def turn_on():
    global server_status
    with open(output_file_path, 'w') as output_file:
        output_file.write("")
    pull_and_restart()  # Вызываем функцию pull_and_restart перед стартом сервера
    start_server()
    return 'Включено'

@app.route('/turn_off')
def turn_off():
    global server_status
    stop_server()
    return 'Выключено'

@app.route('/get_status')
def get_status():
    return server_status

@app.errorhandler(500)
def internal_server_error(error):
    return f'Internal Server Error: {error}', 500

if __name__ == '__main__':
    print("Start Web-Server Program")

    # Запускаем Flask в отдельном потоке
    flask_thread = Thread(target=run_flask)
    flask_thread.start()

    # Добавляем бесконечный цикл для автоматического обновления каждые 5 минут
    while True:
        try:
            pull_and_restart()
            print("Pull and restart successful.")
        except subprocess.CalledProcessError as e:
            print(f"Error: {e}")

        time.sleep(15)  # 5 минут
