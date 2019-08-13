import os, subprocess, sys, shutil

USAGE = 'usage: python3 tools.py [build | buildDebug | clean | test | valgrind | help]'

def get_mode(debugging):
    if debugging:
        return 'debug'
    else:
        return 'release'

def build_folder(debugging):    
    return os.path.join('build', get_mode(debugging) + '_' + os.name)

def executable(debugging):
    folder = build_folder(debugging)
    
    if os.name == 'nt':
        ext = '.exe'
    elif os.name == 'posix':
        ext = ''
    
    return os.path.join(folder, 'blerg') + ext

def debug_flag(debugging):
    if debugging:
        return '-g'
    else:
        return ''

def include_cmd(src, debugging=False):
    return 'g++ %s -I"include" -MM -c %s' %(debug_flag(debugging), src)

def compile_cmd(src, obj, debugging=False):
    return 'g++ %s -I"include" -c %s -o %s' %(debug_flag(debugging), src, obj)

def link_cmd(executable, objs, debugging=False):
    return 'g++ %s -o %s %s' %(debug_flag(debugging), executable, ' '.join(objs))

def get_files(dir):
    """
    A generator that yields the files in a given directory
    """
    for dirpath, dirnames, filenames in os.walk(dir):
        for filename in filenames:
            yield os.path.join(dirpath, filename)

def with_extension(files, extension):
    """
    Filters the files to only keep those with the given extension
    """
    return filter(lambda x: x.endswith(extension), files)

def obj_tuple(files, debugging):
    """
    Turns a iterable of source files and produces a iterable of a tuple of
    source files and object files
    """
    def obj_file(file):
        parts = file.split('.')
        parts = parts[:-1] #everything but the end, removes the extension
        end = os.path.relpath(''.join(parts), 'src')
        return os.path.join(build_folder(debugging), end) + '.o'
    return map(lambda x: (x, obj_file(x)), files)

class ChangedStatus:
    CHANGED = 'changed'
    UNCHANGED = 'unchanged'
    ERROR = 'error'

def changed_sources(files, debugging=False):
    """
    Returns the source files that need recompilation
    """
    def is_changed(file):
        #check if object file exists
        if not os.path.exists(file[1]):
            return ChangedStatus.CHANGED
        
        #check if the source file changed
        mtime = os.stat(file[1]).st_mtime
        if os.stat(file[0]).st_mtime > mtime:
            return ChangedStatus.CHANGED
        
        #check if any of the header files
        cmd = include_cmd(file[0], debugging)
        result = subprocess.run(cmd, stdout=subprocess.PIPE, shell=True)
        if result.returncode != 0:
            return ChangedStatus.ERROR
        
        #convert bytes to str and get the prerequisites
        includes = result.stdout.decode('utf-8').split(':')[1]
        #remove newline handling
        includes = includes.replace('\\\n', ' ').replace('\\\r', ' ')
        includes = includes.replace('\n', ' ').replace('\r', ' ')
        #split the prerequisites
        includes = includes.split(' ')
        #filter out the empty items
        includes = list(filter(lambda x: len(x) != 0, includes))
        
        if any_newer_than(includes, file[1]):
            return ChangedStatus.CHANGED
        else:
            return ChangedStatus.UNCHANGED
    
    for file in files:
        status = is_changed(file)
        if status == ChangedStatus.CHANGED or status == ChangedStatus.ERROR:
            yield (status, file)

def any_newer_than(a, b):
    """
    Returns true iff any of the files in a are newer than file b
    """
    #if b does not exist, then recompilation is necessary
    if not os.path.exists(b):
        return True
    mtime = os.stat(b).st_mtime
    for i in a:
        if os.stat(i).st_mtime > mtime:
            return True
    return False

def build(debugging=False):
    """
    builds the executable
    """
    files = list(obj_tuple(with_extension(get_files('src'), '.c'), debugging))

    failed = False
    for status, (src, obj) in changed_sources(files, debugging):
        if status == ChangedStatus.ERROR:
            print('error in %s' %src)
            failed = True
        elif status == ChangedStatus.CHANGED:
            dir = os.path.dirname(obj)
            if not os.path.exists(dir):
                os.makedirs(dir)
            print('compiling %s' %src)
            result = subprocess.run(compile_cmd(src, obj, debugging), shell=True)
            if result.returncode != 0:
                print('failed to compile %s' %src)
                failed = True
    

    if not failed:
        obj_files = list(map(lambda x: x[1], files))
        if any_newer_than(obj_files, executable(debugging)):
            print('linking')
            cmd = link_cmd(executable(debugging), obj_files, debugging)
            result = subprocess.run(cmd, shell=True)
            if result.returncode != 0:
                print('link failure')
                failed = True
    
    return not failed

def clean():
    shutil.rmtree('build')

def test():
    if build(True):
        if os.name == 'nt':
            subprocess.run(r'build\debug_nt\blerg.exe --test', shell=True)
        elif os.name == 'posix':
            subprocess.run('build/debug_posix/blerg --test', shell=True)

def valgrind():
    if os.name == 'nt':
        print('cannot run valgrind in a windows environment')
    elif os.name == 'posix':
        if build(True):
            subprocess.run('valgrind --leak-check=full ./build/debug_posix/blerg --test', shell=True)

def help():
    print(USAGE)

def invalid_args():
    print('invalid arguments')
    print(USAGE)

def main():
    if os.name not in ['nt', 'posix']:
        print('only nt and posix environments are supported')
        print('the program detected that the environment is %s' %os.name)
        return
    
    if len(sys.argv) == 1:
        build()
    elif len(sys.argv) != 2:
        invalid_args()
    elif sys.argv[1] == 'build':
        build()
    elif sys.argv[1] == 'buildDebug':
        build(True)
    elif sys.argv[1] == 'clean':
        clean()
    elif sys.argv[1] == 'test':
        test()
    elif sys.argv[1] == 'valgrind':
        valgrind()
    elif sys.argv[1] == 'help':
        help()
    else:
        invalid_args()

if __name__ == '__main__':
    main()