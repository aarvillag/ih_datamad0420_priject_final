import streamlit as st
# To make things easier later, we're also importing numpy and pandas for
# working with sample data.
import numpy as np
import pandas as pd
import datetime
import time
from datetime import date
from datetime import datetime
import random
import collections
import matplotlib.pyplot as plt
import matplotlib
import matplotlib.dates as mdates
from all_function import max_dd_evol, max_dd, time_guarantee, short_long_operationLSEX, validate_file
import subprocess

################################################################################################
### SIDEBAR & SELECTION

st.title('     Investment Systems Tool')
st.title('With Futures Intraday Strategies')

st.sidebar.text('Select a future contract:\n')
s_contract = st.sidebar.select_slider('(ES = SP500 | NQ = NASDAQ | YM = DOW JONES | CL = CRUDE OIL | RLY = RUSSEL2000)',
     options=['ES', 'NQ', 'YM', 'CL', 'RLY'],
     value=('ES'))

st.sidebar.text('Select a timetable to operate:\n')
s_timetable = st.sidebar.select_slider('( Asia = 01:00-08:30 | Europe = 09:00-18:30 | America = 15:00-22:30)',
     options=['01:00-08:30', '09:00-18:30', '15:00-22:30'],
     value=('09:00-18:30'))

s_rng_years = st.sidebar.select_slider('Select quantity years for Range Estimation',
     options=[2, 3, 4],
     value=(4))

s_rng_adjust = st.sidebar.select_slider('Select percentage for Range Adjustment',
     options=[65, 70, 75, 80],
     value=(70))

s_axs_target = st.sidebar.select_slider('Select a qty of axis for Target',
     options=[2, 3, 4, 5],
     value=(2))

st.sidebar.text('Select an analysis period:\n')
s_year_ini = st.sidebar.select_slider('to start',
     options=[2014, 2015, 2016, 2017, 2018, 2019, 2020],
     value=(2014))

s_year_end = st.sidebar.select_slider('to finish',
     options=[2014, 2015, 2016, 2017, 2018, 2019, 2020],
     value=(2020))

if s_year_end < s_year_ini:
    st.warning('Please year to finish analysis wrong. It must be equal or greater than year to start analysis')
    st.stop
    
strategies_tab_on = st.sidebar.checkbox('Generate a sumary table with all strategies that you check')

reset_strg_tab_on = st.sidebar.checkbox('Reset the table with registered strategies(delete dataset)')

execute_preproc_on = st.sidebar.checkbox('Execute the preprocess for all options')



my_t0=pd.DataFrame(columns=['Contract','Timetable', 'Range Estimation', 'Range Adjustment', 'Target in axis','Start Year', 'End Year'])
my_t0.loc['Your Selection'] = [s_contract,s_timetable,s_rng_years,s_rng_adjust,s_axs_target,s_year_ini,s_year_end]

def color_change(val):
    """
    Takes a scalar and returns a string with
    the css property `'color: red'` for negative
    strings, black otherwise.
    """
    color = 'blue'
    return 'color: %s' % color    

st.table(my_t0.style.applymap(color_change))

################################################################3

## FROM SELECTION
# Upload the file with my data already preprocessed

min_tic=0.25
axs_limit= 1
zero = '0'
key_config = s_contract+s_timetable[0:2]+s_timetable[3:5]+s_timetable[6:8]+s_timetable[9:]
key_config = key_config + str(s_rng_years)+str(s_rng_adjust)

tmp=subprocess.run(['../RangingOperating', s_contract, s_timetable, str(s_rng_years), str(s_rng_adjust)], capture_output=False, stdout=None)

if tmp.returncode != 9:
    st.write(f'Error = File not processed({tmp.returncode})')
else:
    st.text(f'file processed {key_config}')

my_file=f'../Data/{key_config}_03.csv'

if not validate_file(my_file):
    st.warning('File preprocessed does not exist. Please be sure to process it before to have the information')
    st.stop

df_day=pd.read_csv(my_file,dtype={'date': str,'timetable': str,'max' : float ,'min' : float ,'range' : float ,
                                              'range-trail' : float, 'range-avg' : float, 'open': float, 'long-close': float,
                                              'short-close': float, 'close': float, 'long-rst': float, 'short-rst': float,
                                              'long-acc': float, 'short-acc': float},
                   parse_dates=['date'])  

# Step 0 for generate the short/long process and the result

my_multiplier, my_guarantee, my_delta_time, my_tic = time_guarantee(s_contract, s_year_ini, s_year_end, df_day)


#df_result = short_long_operationLSEX(s_contract, s_year_ini, s_year_end, df_day, axs_limit, s_axs_target, my_multiplier)

#df_result.to_csv(f'../Data/{s_contract}_04.csv', index=False)                                      # XX_04


tmp=subprocess.run(['../LongShort', s_contract, str(s_year_ini), str(s_year_end), my_file, str(axs_limit), str(s_axs_target), str(my_multiplier), str(my_tic), 'LSEX'],
                  capture_output=False, stdout=None)

if tmp.returncode != 9:
    st.write(f'Error = File not processed({tmp.returncode})')


my_file=f'../Data/{s_contract}_04.csv'
df_result=pd.read_csv(my_file,dtype={'date': str,'timetable': str,'max' : float ,'min' : float ,'range' : float ,
                                              'range-trail' : float, 'range-avg' : float, 'open': float, 'long-close': float,
                                              'short-close': float, 'close': float, 'long-rst': float, 'short-rst': float,
                                              'long-acc': float, 'short-acc': float, 'operation': str, 'target': float, 'limit': float,
                                              'result-pts': float, 'result-accum-pts': float, 'contract-qty': float, 'multiplier': float,
                                              'result-amt': float, 'result-accum-amt': float, 'long-accum-tst': float, 'short-accum-tst': float},
                     parse_dates=['date'])  

my_result = df_result[(df_result['result-amt'] != 0) & (df_result['result-accum-amt'] != 0)]['result-amt']
date_serie = df_result[(df_result['result-amt'] != 0) & (df_result['result-accum-amt'] != 0)]['date']

## step 1

df=df_result

# table with monthly summary

st.subheader('Monthly Profit %')

df_sumry=pd.DataFrame((my_result.cumsum()+my_guarantee).tolist(), index=date_serie, columns=['capital'])
df_sumry_res=df_sumry.resample('M').last().pct_change()

my_column='capital'
my_column1='total'
my_years=[]
for i in df_sumry_res.index:
    my_year=i.year
    if my_year not in my_years:
        my_years.append(my_year)
        
df_sumry_tab=pd.DataFrame(0.0, columns=[1,2,3,4,5,6,7,8,9,10,11,12,'total'], index=my_years)
df_sumry_tab1=df_sumry_tab
cont=0
for i in df_sumry_res.index:
    df_sumry_tab[i.month][i.year]=df_sumry_res[my_column][i]
    df_sumry_tab1[i.month][i.year]=f'{df_sumry_tab[i.month][i.year]*100:4.1f}'
    if cont==0:
        a=df_sumry.resample('M').last()
        df_sumry_tab[i.month][i.year]=(a[my_column].iloc[0]-my_guarantee)/my_guarantee
        df_sumry_tab1[i.month][i.year]=f'{df_sumry_tab[i.month][i.year]*100:4.1f}'
    cont=cont+1

b = df_sumry.resample('A').last()
c = df_sumry.resample('A').last().pct_change()
for i in range(len(df_sumry_tab.index)):                           # totalizing figures annually
    j = c.index.year[i]
    if i == 0:
        df_sumry_tab['total'][j] = ((b[my_column].iloc[i] - my_guarantee) / my_guarantee)
        df_sumry_tab1['total'][j] = f'{((b[my_column].iloc[i] - my_guarantee) / my_guarantee)*100:4.1f}'
    else:
        df_sumry_tab['total'][j] = c[my_column].iloc[i]
        df_sumry_tab1['total'][j] = f'{c[my_column].iloc[i]*100:4.1f}'
    
def color_negative_red(val):
    """
    Takes a scalar and returns a string with
    the css property `'color: red'` for negative
    strings, black otherwise.
    """
    color = 'red' if val < 0 else 'blue'
    return 'color: %s' % color    

s = df_sumry_tab1.style.applymap(color_negative_red).set_precision(1)
    
st.table(s)

# step 2 monte carlo simulation
         
mc=np.random.choice(my_result,size=(my_result.count(),1000), replace=True)
df_mc=pd.DataFrame(mc)
df_mc['original']=my_result.tolist()


# step 3 statistics of montecarlo

pd.options.mode.chained_assignment = None                     # default='warn' This is to avoid warnings of chained-assignment
guarantee=my_guarantee                # dos veces la garantia del broker (5-10% del valor subyacente)
time=my_delta_time                     # tiempo en años

feature_list = ['random', 'netprofit', 'netprofit_prc', 'annual_netprofit_prc', 'MDD', 'MDD_prc', 'exposure','ANP-MDD', 'sharpe', 'recoveryfactor',                               'profitfactor', 'winners_prc', 'avg_profit','losers_prc', 'avg_lost', 'lowest_cap']

df_stat = pd.DataFrame(0.0, index=np.arange(1001), columns=feature_list)

for w in range(1001):
    jaj=w
    if w==1000:
        jaj='original'
    df_stat['random'][w]=jaj
    df_stat['netprofit'][w]=df_mc[jaj].sum()
    df_stat['netprofit_prc'][w]=(df_mc[jaj].sum()+guarantee)/guarantee
    if df_stat['netprofit_prc'][w]>0:
        df_stat['annual_netprofit_prc'][w] = ((df_mc[jaj].sum()+guarantee)/guarantee)**(1/time) - 1
    else:
        df_stat['annual_netprofit_prc'][w] = 0.0
    df_stat['MDD'][w], df_stat['MDD_prc'][w] = max_dd(guarantee, df_mc[jaj])
    df_stat['exposure'][w]=38/88
    if df_stat['netprofit_prc'][w]>0:
        df_stat['ANP-MDD'][w]=df_stat['annual_netprofit_prc'][w]/df_stat['MDD'][w]
    else:
        df_stat['ANP-MDD'][w]=0.0
    df_stat['sharpe'][w]=df_mc[jaj].cumsum().diff().mean()/df_mc[jaj].cumsum().diff().std() * np.sqrt(252)
    df_stat['recoveryfactor'][w]=df_stat['netprofit'][w]/df_stat['MDD'][w]
    df_stat['profitfactor'][w]=(0-1)*np.nansum(np.where(df_mc[jaj]>=0,df_mc[jaj],np.nan))/np.nansum(np.where(df_mc[jaj]<0,df_mc[jaj],np.nan))
    df_stat['winners_prc'][w]=df_mc[df_mc[jaj]>=0][jaj].count()/df_mc[jaj].count()
    df_stat['avg_profit'][w]=np.nanmean(np.where(df_mc[jaj]>=0,df_mc[jaj],np.nan))
    df_stat['losers_prc'][w]=df_mc[df_mc[jaj]<0][jaj].count()/df_mc[jaj].count()
    df_stat['avg_lost'][w]=np.nanmean(np.where(df_mc[jaj]<0,df_mc[jaj],np.nan))
    df_stat['lowest_cap'] = np.min(df_mc[jaj].sum()+guarantee)

# step 4 Quartiles dataframe

df_quart=pd.DataFrame(0.0, columns=range(16),index=range(101))
for k in range(1,16):
    df_quart[k]=np.percentile(df_stat[df_stat.columns[k]].tolist(), range(101))
df_quart.columns=feature_list

####################################################################################3

# PROFIT & CAPITAL

st.subheader('Indicators & Ratios')

guarantee=my_guarantee
time=my_delta_time

init_capital=guarantee
a0=f'{init_capital:10.2f}$'
final_cum_cap=df_mc['original'].sum()+guarantee
a1=f'{final_cum_cap:10.2f}$'
final_net_cap=df_mc['original'].sum()
a2=f'{final_net_cap:10.2f}$'
min_capital=df_mc['original'].cumsum().min() + guarantee
a3=f'{min_capital:10.2f}$'
max_capital=df_mc['original'].cumsum().max() + guarantee
a4=f'{max_capital:10.2f}$'
 
my_t1=pd.DataFrame(columns=['Initial','Final cum','Final net', 'Minimum', 'Maximum'], index=['Capital'])
my_t1.loc['Capital'] = [a0,a1,a2,a3,a4]
st.table(my_t1.style.applymap(color_change))

net_profit_prc=(df_mc['original'].sum()+guarantee)/guarantee
b0=f'{net_profit_prc*100:10.2f}%'
annual_profit_prc=((df_mc['original'].sum()+guarantee)/guarantee)**(1/time) - 1
b1=f'{annual_profit_prc*100:10.2f}%'

max_drawdown, max_drawdown_pct = max_dd(my_guarantee, my_result)
c0=f'{max_drawdown:10.2f}$'
c1=f'{max_drawdown_pct*100:10.2f}%'

mycol01, mycol02 = st.beta_columns([1,1])

with mycol01:
    my_t2=pd.DataFrame(columns=['Total Net','Total Annual'])
    my_t2.loc['Profit'] = [b0,b1]
    st.table(my_t2.style.applymap(color_change))

with mycol02:
    my_t3=pd.DataFrame(columns=['Amount','Percentage'])
    my_t3.loc['Max Drawdown'] = [c0,c1]
    st.table(my_t3.style.applymap(color_change))

sharpe = my_result.cumsum().diff().mean() / my_result.cumsum().diff().std() * np.sqrt(252)
d0=f'{sharpe:10.2f}'
recovery_factor = my_result.sum() / max_drawdown
d1=f'{recovery_factor:10.2f}'
profit_factor = (0 - 1) * np.nansum(np.where(my_result >= 0, my_result, np.nan)) / \
                    np.nansum(np.where(my_result < 0, my_result, np.nan))   
d2=f'{profit_factor:10.2f}'
sortino = my_result.mean()/np.sqrt(np.mean(my_result[my_result < 0].to_numpy()**2)) * np.sqrt(252)
d3=f'{sortino:10.2f}'

winners_pct = my_result[my_result >= 0].count()/my_result.count()
e0=f'{winners_pct*100:10.2f}%'
losers_pct = my_result[my_result < 0].count()/my_result.count()
e1=f'{losers_pct*100:10.2f}%'
win_avg = my_result[my_result >= 0].sum()/my_result[my_result >= 0].count()
e2=f'{win_avg:10.2f}$'
lost_avg = my_result[my_result < 0].sum()/my_result[my_result < 0].count()
e3=f'{lost_avg:10.2f}$'

# Maximum negatives traders in a row
trades = my_result.tolist()
neg_count = 0
max_neg_count = 0
for n in range(len(trades)):
    if trades[n] < 0:
        neg_count += 1
    else:
        neg_count = 0
    if neg_count > max_neg_count:
        max_neg_count = neg_count

mycol03, mycol04 = st.beta_columns([1,1])

with mycol03:
    my_t4=pd.DataFrame(columns=['Ratio'])
    my_t4.loc['Sharpe'] = [d0]
    my_t4.loc['Sortino'] = [d3]
    my_t4.loc['Recovery Factor'] = [d1]
    my_t4.loc['Profit Factor'] = [d2]
    st.table(my_t4.style.applymap(color_change))

with mycol04:
    my_t5=pd.DataFrame(columns=['Avg Amount','Percentage'])
    my_t5.loc['Winners'] = [e2,e0]
    my_t5.loc['Losers'] = [e3,e1]
    st.table(my_t5.style.applymap(color_change))
    my_t7=pd.DataFrame(columns=['Max qty'])
    my_t7.loc['Negatives Trades In A Row'] = [max_neg_count]
    st.table(my_t7.style.applymap(color_change))
###########################################################################################3

st.subheader('Graphs')

plt.rcParams.update({'font.size': 16})

col01, col02 = st.beta_columns([1, 1])

with col01:                                     # plot capital evolution
    my_plot_1 = pd.DataFrame((my_result.cumsum() + my_guarantee).tolist(), columns=['original'],
                            index=date_serie.tolist())
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_1['original'], lw=3, color="b", alpha=.8, label="Original")
    ax.fill_between(date_serie.tolist(), my_plot_1['original'], my_guarantee, facecolor='green',
                     where=my_plot_1['original'] >= my_guarantee)
    ax.fill_between(date_serie.tolist(), my_plot_1['original'], my_guarantee, facecolor='red',
                     where=my_plot_1['original'] < my_guarantee)
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Capital Evolution', fontweight="bold", fontsize=30)
    plt.ylabel("Results in $", fontsize=20)
    plt.xlabel('Dates', fontsize=20)
    st.pyplot(fig)
    plt.close()

with col02:                                      # plot max drawdown percentage
     # step 1 calculate de drawdown evolution in a dataframe
    my_dd = max_dd_evol(my_guarantee, my_result)
     # step 2 generate the dataframe and plot
    my_plot_3 = pd.DataFrame((my_dd['pct_dd'] * (0 - 100)).tolist(), columns=['drawdown'],
                             index=date_serie.tolist())
    my_plot_3['max_drawdown'] = (my_dd['pct_max_dd'] * (0 - 100)).tolist()
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.plot(my_plot_3['drawdown'], lw=1, color="black", alpha=.8, label="drawdown")
    ax.plot(my_plot_3['max_drawdown'], lw=3, color="red", alpha=.8, label="max-drawdown")
    ax.fill_between(date_serie.tolist(), my_plot_3['drawdown'], 0, facecolor='yellow')
    ax.axhline(0, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Drawdown Evolution', fontweight="bold", fontsize=30)
    plt.ylabel("Drawdown in %", fontsize=20)
    plt.xlabel('Dates', fontsize=20)
    st.pyplot(fig)

col05, col06 = st.beta_columns([1, 1])
    
with col05:
    fig, ax = plt.subplots(figsize=(12, 8))
    data = (my_result.cumsum() + my_guarantee).pct_change().tolist()

    num, bins, patches = ax.hist(data, edgecolor='white', linewidth=1,
                               bins=[-0.10, -0.05, 0.0, 0.05, 0.10, 0.15, 0.2, 0.25, 0.3])
    for i in range(8):
        if (num[i] > 0) & (bins[i] >= 0) & (bins[i+1] >= 0):
            patches[i].set_facecolor('green')
        elif num[i] > 0:
            patches[i].set_facecolor('red')
    ax.set_title('Profit Distribution', fontweight="bold", fontsize=30)
    plt.ylabel("Trades", fontsize=20)
    plt.xlabel("% Profit", fontsize=20)
    ax.legend()
    ax.grid(True)
    st.pyplot(fig)

# table with percentils from Monte Carlo Simulation
    
my_t6=pd.DataFrame(columns=['Final Capital', 'Annual Profit%', 'Max Drawdown$', 'Max Drawdown%', 'Lowest Capital'])
index = df.index
index.name = "Percentiles"

for i in [1,5,10,25,50,75,90,95,99]:
    my_t6.loc[f'{i}%'] = [f"{df_quart['netprofit'][i]+my_guarantee:8.0f}",
                          f"{df_quart['annual_netprofit_prc'][i]*100:5.2f}",
                          f"{df_quart['MDD'][i]:8.0f}", 
                          f"{df_quart['MDD_prc'][i]:5.2f}",
                          f"{df_quart['lowest_cap'][i]:8.0f}"]
st.table(my_t6)


b1=f'{annual_profit_prc*100:10.2f}%'
    
col03, col04 = st.beta_columns([1,1])

with col03:                                      # plot all montecarlo simulations
    my_idx=date_serie
    my_plot_0=pd.DataFrame(df_mc.iloc[:, 0:1001].to_numpy(), index=my_idx)
    fig, ax = plt.subplots(figsize=(12,8))
    ax.plot(my_plot_0.cumsum()+my_guarantee, lw=1, alpha=.8)
    ax.plot(my_plot_0[1000].cumsum()+my_guarantee, lw=3, color="b", alpha=.8, label="Original")
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.set_title('Monte Carlo Simulation', fontweight="bold", fontsize=30)
    plt.ylabel("Results", fontsize=20)
    plt.xlabel("Dates", fontsize=20)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    st.pyplot(fig)
    plt.close()

with col04:                                  # plot min and max capital evolution
    my_idx=date_serie
    # Step 1 = localize the min and max in dataframe
    lst = df_mc.shape[0] - 1
    val_max = 0.0
    val_min = 999999.9
    my_idx_min = 0
    my_idx_max = 0
    for h in range(len(df_mc.columns) - 1):
        val = df_mc[h].cumsum()[lst]
        if val > val_max:
            val_max = val
            my_idx_max = h
        if val < val_min:
            val_min = val
            my_idx_min = h
    # step 2 we generate the plot
    my_plot_8 = pd.DataFrame((my_result.cumsum()+my_guarantee).tolist(), columns=['original'],
                            index=my_idx)
    my_plot_8[my_idx_min] = (df_mc[my_idx_min].cumsum()+my_guarantee).tolist()
    my_plot_8[my_idx_max] = (df_mc[my_idx_max].cumsum()+my_guarantee).tolist()
    fig, ax = plt.subplots(figsize=(12,8))
    ax.plot(my_plot_8)
    ax.plot(my_plot_8['original'], lw=2, color="b", alpha=.8, label="Original")
    ax.plot(my_plot_8[my_idx_max], lw=2, color="g", alpha=.8, label="Max")
    ax.plot(my_plot_8[my_idx_min], lw=2, color="r", alpha=.8, label="Min")
    ax.axhline(my_guarantee, color="black")
    ax.legend()
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m'))
    ax.grid(True)
    ax.set_title('Capital evolution', fontweight="bold", fontsize=30)
    plt.ylabel("Results in $", fontsize=20)
    plt.xlabel('Dates', fontsize=20)
    st.pyplot(fig)
    plt.close()

plt.rcParams.update({'font.size': 8})
    
colB, colC = st.beta_columns([1, 1])

#with colA:                                            # plot net profit $ percentils
#    my_plot_4 = pd.DataFrame(df_quart['netprofit'].tolist(), columns=['netprofit'])
#    fig, ax = plt.subplots(figsize=(4, 3))
#    ax.plot(my_plot_4['netprofit'], lw=3, color="b", alpha=.8, label="Net Profit")
#    ax.axhline(0, color="black")
#    ax.legend()
#    ax.grid(True)
#    ax.set_title('Net Profit / Percentils', fontweight="bold")
#    plt.ylabel("Net Profit in $")
#    plt.xlabel('Percentils')
#    st.pyplot(fig)
#    plt.close()

with colB:                                          # plot net profit $ percentils
    my_plot_5 = pd.DataFrame((df_quart['annual_netprofit_prc'] * 100).tolist(), columns=['netprofit'])
    fig, ax = plt.subplots(figsize=(4, 3))
    ax.plot(my_plot_5['netprofit'], lw=3, color="b", alpha=.8, label="Annual Net Profit %")
    ax.axhline(0, color="black")
    ax.legend()
    ax.grid(True)
    ax.set_title('Annual Net Profit Percentage / Percentils', fontweight="bold")
    plt.ylabel("Annual Net Profit %")
    plt.xlabel('Percentils')
    st.pyplot(fig)
    plt.close()

with colC:                                               # plot Maximum Drawdown Percentage / Percentils
    my_plot_6 = pd.DataFrame((df_quart['MDD_prc'] * (0 -100)).tolist(), columns=['MDD'])
    fig, ax = plt.subplots(figsize=(4, 3))
    ax.plot(my_plot_6['MDD'], lw=3, color="b", alpha=.8, label="Max Drawdown %")
    ax.axhline(0, color="black")
    ax.legend()
    ax.grid(True)
    ax.set_title('Max. Drawdown Percentage / Percentils', fontweight="bold")
    plt.ylabel("Max Drawdown %")
    plt.xlabel('Percentils')
    st.pyplot(fig)
    plt.close()
    
################################################################################
# SUMARY TABLE WITH STRATEGIES REGISTERED.



if strategies_tab_on:
    
    my_table='../Data/TableStrategiesReg.csv'

    if not validate_file(my_file):
        st.warning('File with strategies sumary does not exist.')
        st.stop

    df_strg_sumry=pd.read_csv(my_table,dtype={'id': str,'datetime': str,'contract': str,'timetable' : str ,'Rng Estim' : str ,
                                              'Rng Adj' : str , 'Axis Target' : str, 'period' : str, 'Annual Profit%': str, 
                                              'Max Drawdown%': str, 'Winners%': str, 'Recovery Factor': str, 'Profit Factor': str,
                                              'Sharpe': str})

    my_key = key_config + str(s_year_ini) + str(s_year_end) + str(s_axs_target)
    
    if df_strg_sumry[df_strg_sumry['id']==my_key]['id'].count()==0:
        today_is=datetime.now().strftime('%Y-%m-%d %H:%M:%S')    
        next_row=df_strg_sumry.shape[0]
        df_strg_sumry.loc[next_row]=[my_key,today_is,s_contract,s_timetable,str(s_rng_years),str(s_rng_adjust),str(s_axs_target),
                                     str(s_year_ini)+'-'+str(s_year_end),b1,c1,e0,d1,d2,d0]

    if reset_strg_tab_on:
        last=df_strg_sumry.shape[0]
        df_strg_sumry.drop(df_strg_sumry.index[range(last)], inplace = True)
        reset_strg_tab = False
    
    st.subheader('Registered Strategies')
    st.dataframe(df_strg_sumry.T)
    
    df_strg_sumry.to_csv(my_table,index=False)
