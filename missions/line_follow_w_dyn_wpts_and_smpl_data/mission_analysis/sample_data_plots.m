%%% cmoran
%%% UCSB Coastal Oceanography and Autonomous Systems Laboratory
%%% sample_data_plots.m
%%% 13 February 2019

% Script for looking at some of critical data from a mission profile
clear

m = csvread("../output.csv");
n = csvread("../padded_output.csv");
p = csvread("../maximums_output.csv");
q = csvread("../position_output.csv");

[x y] = size(m);
% Pulls in the image csv file and creates image using pcolor

figure(1)
subplot(2,3,1)
h = pcolor(m)
set(h, 'EdgeColor', 'none');
colormap gray
title('Raw Sample Data Image')
hold on

% Plots a horizontal bar graph of maximum index locations, aligned underneath the
% raw sample data image
subplot(2,3,4)
j = barh(p(:,1))
xlim([0 y])
title('Maximum Return Index Locations')

% Plots the
subplot(2,3,[2 5])
i = pcolor(n)
set(i, 'EdgeColor', 'none');
title('Padded + Oscillated Sample Data Image')
%colormap jet

% creates a histogram matrix
hst = zeros(length(p(:,1)),2);
for a = 1:length(hst(:,1))
  hst(a,1) = a;
  hst(p(a,1),2) = hst(p(a,1),2) + 1;
end

subplot(2,3,[3 6])
bar(hst(:,1),hst(:,2))
xlim([0 y])
title('Histogram of Indices for Max Ping Return Value')
xlabel('Ping Index [pixel]')
ylabel('Count Number')

pause


for k = 1:length(q-1)
  roc(k) = (q(k) - q(k+1));
end


figure(2)
% nav_x, nav_y, nav_heading, max_index, distance_from_pixels
% ideal_distance is set to 10.5
subplot(2,3,[1 2 4 5])
scatter(q(:,1),q(:,2),'b') % plots position during run
hold on
line([50,50],ylim,'Color','black','LineWidth',2)
line([150,150],ylim,'Color','black','LineWidth',2)
title('Vehicle Position (x,y)')

subplot(2,3,3)
plot(1:length(q(:,3)),q(:,3),'b-','Linewidth',2) % plots heading during run
hold on
line(xlim,[90 90],'Color','black','LineWidth',1)
hold on
line(xlim,[270 270],'Color','black','LineWidth',1)
title('Vehicle Heading')
xlabel('time (s)')
ylabel('[degrees]')

for k = 1:length(q-1)
  roc(k) = (q(k) - q(k+1));
end
subplot(2,3,6)
plot(1:length(roc),roc,'b-','Linewidth',1) % plots mvg avg variation of heading during run
title('Variation of Vehicle Heading')
xlabel('time (s)')
ylabel('[degrees]')

pause
