import matplotlib.pyplot as plt

def read_file():
    f=open("tcp-example.tr","r")
    lines=[]
    while f:
        line=f.readline()
        if line=='':
            break
        lines.append(line)
    return lines

lines = read_file()

mapping={}

required_str = '/NodeList/1/DeviceList/1'

def get_points():
    x_axis_vals=[]
    y_axis_vals=[]
    for line in lines:
        line_split=line.strip().split()
        if line_split[2][0:24]==required_str:
            if len(line_split)>=36:
                if line_split[0]=='+':
                    mapping[int(line_split[36][4:])]=float(line_split[1])
                if line_split[0]=='-':
                    time=mapping[int(line_split[36][4:])]
                    delay=float(line_split[1])-time
                    x_axis_vals.append(float(line_split[1]))
                    y_axis_vals.append(delay)
    return x_axis_vals, y_axis_vals

x_axis_vals, y_axis_vals = get_points()

def plot_graph():
    plt.plot(x_axis_vals, y_axis_vals)
    plt.xlabel("Time in seconds")
    plt.ylabel("Queuing delay in seconds")
    plt.savefig('queueing_delay_plot.eps', format='eps')

plot_graph()
