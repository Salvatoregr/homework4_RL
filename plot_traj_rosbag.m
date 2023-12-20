clear all
close all
clc

bagFilePath = 'robot_position_.bag';

bag = rosbag(bagFilePath);
bag.AvailableTopics
desiredTopic = '/fra2mo_position';


bagSelection = select(bag, 'Topic', desiredTopic);

msgs = readMessages(bagSelection, 'DataFormat', 'struct');

figure;

for i = 1:numel(msgs)
    % Estrai le coordinate X, Y, Z direttamente dal messaggio
    X = msgs{i}.X;
    Y = msgs{i}.Y;
    Z = msgs{i}.Z;
    
    % Plotta la posizione
    plot3(X, Y, Z, 'b.');
    hold on;
end

xlabel('X');
ylabel('Y');
zlabel('Z');
title('Posizione da /fra2mo\_position');
hold off;
