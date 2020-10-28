import pandas as pd
import numpy as np
import datetime
from datetime import date
from p_ranging import m_dataranging as mda               # import from pack ranging the module


def market_operation(df_int, df_day, timetable_ini, timetable_end):
    """
    This function complete a dataframe df_day with open, close-long, close-short for a daily base in
    a specific timetable
    You can realise that we are going to operate long and short.
    On the other hand we will use the order STOP-TRAIL
    -Parameters
    ----------
    df_int : dataframe with original data (intraday movements of a future contract)
            ['date', 'time', 'open', 'max', 'min', 'close', 'volume']
    df_day : new dataframe to create with columns ['date', 'timetable', 'max', 'min', 'range', range-avg, range-trail].
            We will add the following columns
             ['open', 'close-long', 'close-short', 'close', long-rst, short-rst, long-acc, short-acc]
    timetable_ini : time the operation start everyday. Example='09:00'
    timetable_end : time the operation finish everyday. Example='18:00'

    -Returns
    -------
    nothing
    The function fill the dataframe df_day with daily
    ['open', 'long-close', 'short-close', 'close', 'long-rst', 'short-rst', 'long-acc', 'short-acc']
    """

    pd.options.mode.chained_assignment = None  # default='warn' This is to avoid warnings of chained-assignment

    # Create new columns in the dataframe and fill them with zeros
    df_day['open'] = np.zeros(len(df_day)).astype('float')
    df_day['long-close'] = np.zeros(len(df_day)).astype('float')
    df_day['short-close'] = np.zeros(len(df_day)).astype('float')
    df_day['close'] = np.zeros(len(df_day)).astype('float')
    df_day['long-rst'] = np.zeros(len(df_day)).astype('float')
    df_day['short-rst'] = np.zeros(len(df_day)).astype('float')
    df_day['long-acc'] = np.zeros(len(df_day)).astype('float')
    df_day['short-acc'] = np.zeros(len(df_day)).astype('float')

    pt_ini_date = df_day['date'][df_day[df_day['range-trail'] != 0].index[0]].year * 10000 + \
                df_day['date'][df_day[df_day['range-trail'] != 0].index[0]].month * 100 + \
                df_day['date'][df_day[df_day['range-trail'] != 0].index[0]].day

    pt_end_date = df_day['date'][df_day[df_day['range-trail'] != 0].index[-1]].year * 10000 + \
                  df_day['date'][df_day[df_day['range-trail'] != 0].index[-1]].month * 100 + \
                  df_day['date'][df_day[df_day['range-trail'] != 0].index[-1]].day

    pt_ini = mda.locate_1st_day('D', pt_ini_date, df_int, 'datetime_EU')
    pt_end = mda.locate_1st_day('D', pt_end_date, df_int, 'datetime_EU')

# testing dataframe is create only for testing purpose. We will delete this part after good perform confirmation
    testing = pd.DataFrame(
        columns=['datetime_EU', 'open', 'max', 'min', 'close', 'long-stop', 'short-stop', 'long-exit', 'short-exit',
                 'long-operation', 'short-operation', 'max-day', 'min-day', 'range-trail'])
    testing['open'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['max'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['min'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['close'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['long-stop'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['short-stop'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['long-exit'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['short-exit'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['long-operation'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['short-operation'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['max-day'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['min-day'] = np.zeros(pt_end - pt_ini + 1).astype('float')
    testing['range-trail'] = np.zeros(pt_end - pt_ini + 1).astype('float')

    # Define variables to use
    operation = 0  # (1 in market, 0 out market)
    entry = 0.0
    long_stop = 0.0
    short_stop = 0.0
    long_exit = 0.0
    short_exit = 0.0
    long_operation = 0
    short_operation = 0
    maximum = 0.0
    minimum = 0.0
    market_exit = 0.0

    hour_ini = int(timetable_ini[0:2])  # initial values
    minu_ini = int(timetable_ini[3:])
    hour_end = int(timetable_end[0:2])
    minu_end = int(timetable_end[3:])
    tmtb_ini = hour_ini * 100 + minu_ini  # integer value
    tmtb_end = hour_end * 100 + minu_end  # integer value

    my_int_date_ant = 0  # define my date ant

    my_long_acc = 0.0  # long result accumulator
    my_short_acc = 0.0  # short result accumulator
    j = 0  # counter for testing

    for ind in range(pt_ini, pt_end + 1):  # the main loop

        my_int_date = df_int['datetime_EU'][ind].year * 10000 + df_int['datetime_EU'][ind].month * 100 + \
                      df_int['datetime_EU'][ind].day                    # the day

        if my_int_date > my_int_date_ant:                           # This is a new day
            my_int_date_ant = my_int_date

            my_loc = mda.locate_1st_day('D', my_int_date, df_day, 'date')  # locate the day in the df_day
            range_trail = df_day['range-trail'][my_loc]                          # take the range-trail

        my_int_time = df_int['datetime_EU'][ind].hour * 100 + df_int['datetime_EU'][ind].minute  # the time

# checking the bar time is >= than start time of timetable and < than end time of timetable  ==>> operation in market

        if (my_int_time >= tmtb_ini) & (my_int_time < tmtb_end):

            if (my_int_time == tmtb_ini) & (operation == 0):                # The moment to start operation
                entry = df_int['open'][ind]
                long_stop = entry - range_trail
                short_stop = entry + range_trail
                long_exit = df_int['close'][ind]
                short_exit = df_int['close'][ind]
                long_operation = 1
                short_operation = 1
                maximum = df_int['max'][ind]
                minimum = df_int['min'][ind]
                operation = 1

            if operation == 1:
                if short_operation == 1:
                    short_exit = df_int['close'][ind]
                if long_operation == 1:
                    long_exit = df_int['close'][ind]
                if (df_int['max'][ind] > short_stop) & (short_operation == 1):  # if the price overcome short_stop (up)
                    short_exit = short_stop
                    short_operation = 0
                if (df_int['min'][ind] < long_stop) & (long_operation == 1):  # if the price overcome long_stop (down)
                    long_exit = long_stop
                    long_operation = 0
                if (long_stop < df_int['max'][ind] - range_trail) & (long_operation == 1):  # up long stop
                    long_stop = df_int['max'][ind] - range_trail
                if (short_stop > df_int['min'][ind] + range_trail) & (short_operation == 1):  # down short stop
                    short_stop = df_int['min'][ind] + range_trail
                if (my_int_time == tmtb_end) & (long_operation == 1):  # long operation end because finish timetable
                    long_operation = 0
                if (my_int_time == tmtb_end) & (short_operation == 1):  # short operation end because finish timetable
                    short_operation = 0
                if df_int['max'][ind] > maximum:  # update maximum if it is the case
                    maximum = df_int['max'][ind]
                if df_int['min'][ind] < minimum:  # update minimum if it is the case
                    minimum = df_int['min'][ind]
                market_exit = df_int['close'][ind]

        elif (my_int_time == tmtb_end) & (operation == 1):                 # timetable finish and save the result
            df_day['open'][my_loc] = entry
            df_day['long-close'][my_loc] = long_exit
            df_day['short-close'][my_loc] = short_exit
            df_day['close'][my_loc] = market_exit
            df_day['long-rst'][my_loc] = long_exit - entry
            df_day['short-rst'][my_loc] = entry - short_exit
            my_long_acc = my_long_acc + long_exit - entry
            df_day['long-acc'][my_loc] = my_long_acc
            my_short_acc = my_short_acc + entry - short_exit
            df_day['short-acc'][my_loc] = my_short_acc

            operation = 0                                                           # Re-init variables
            entry = 0.0
            long_stop = 0.0
            short_stop = 0.0
            long_exit = 0.0
            short_exit = 0.0
            long_operation = 0
            short_operation = 0
            maximum = 0.0
            minimum = 0.0
            market_exit = 0.0

        # Save tracking information to check the function works only for debuging and testing
        j += 1
        testing['datetime_EU'][j] = df_int['datetime_EU'][ind]
        testing['open'][j] = df_int['open'][ind]
        testing['max'][j] = df_int['max'][ind]
        testing['min'][j] = df_int['min'][ind]
        testing['close'][j] = df_int['close'][ind]
        testing['long-stop'][j] = long_stop
        testing['short-stop'][j] = short_stop
        testing['long-exit'][j] = long_exit
        testing['short-exit'][j] = short_exit
        testing['long-operation'][j] = long_operation
        testing['short-operation'][j] = short_operation
        testing['max-day'][j] = maximum
        testing['min-day'][j] = minimum
        testing['range-trail'][j] = range_trail

    return df_day, testing
