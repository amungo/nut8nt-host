function res = ph_rot2(az,el,L)
res = zeros(1,length(az));
for i = 1:length(az)
    az(i) = mod(az(i),360);
    if ((0 < az(i)) && (az(i) < 90))
        res(i) = (L*cosd(az(i)))/cosd(el(i));
    elseif ((90 < az(i)) && (az(i) < 180))
        res(i) = (L*cosd(180 - az(i)))/cosd(el(i));
    elseif ((180 < az(i)) && (az(i) < 270))
        res(i) = -(L*cosd(az(i) - 180))/cosd(el(i));
    elseif ((270 < az(i)) && (az(i) < 360))
        res(i) = -(L*cosd(360 - az(i)))/cosd(el(i));
    elseif (az(i) == 0 || az(i) == 90 || az(i) == 180 || az(i) == 270)
        res(i) = 0;
    end;
end