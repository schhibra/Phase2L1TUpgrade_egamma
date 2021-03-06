library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

use work.DataType.all;
use work.ArrayTypes.all;

library Int;
use Int.DataType;
use Int.ArrayTypes;

entity Merge16to32 is
port(
    clk : in std_logic := '0';
    a : in Vector(0 to 15) := NullVector(16);
    b : in Vector(0 to 15) := NullVector(16);
    q : out Vector(0 to 31) := NullVector(32)
);
end Merge16to32;

architecture rtl of Merge16to32 is

    constant RouterLatency : integer := 5; -- a guess for now
    signal aPipe : VectorPipe(0 to RouterLatency - 1)(0 to 16 - 1) := NulLVectorPipe(RouterLatency, 16);
    signal b_reg : Vector(0 to 15) := NullVector(16);

    -- Layer input arrays
    -- First index is group, second is within-group
    signal X0 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);
    signal X1 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);
    signal X2 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);

    -- Layer output arrays
    signal Y0 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);
    signal Y1 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);
    signal Y2 : Matrix(0 to 3)(0 to 3) := NullMatrix(4,4);

    -- Global Address arrays
    signal XA0 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal XA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal XA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);

    signal YA0 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal YA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal YA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);

    -- Local Address arrays
    signal XLA0 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal XLA1 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);
    signal XLA2 : Int.ArrayTypes.Matrix(0 to 3)(0 to 3) := Int.ArrayTypes.NullMatrix(4,4);

    -- Final route arrays
    signal bRouted  : Vector(0 to 31) := NullVector(32);
    signal Y : Vector(0 to 31) := NullVector(32);

    -- N is the current base address to route to
    -- M is the max
    --signal N : Int.DataType.tData := Int.DataType.cNull;
    --signal M : Int.DataType.tData := Int.DataType.cNull;
    signal N : integer range 0 to 32 := 0;

begin

    aPipeEnt:
    entity work.DataPipe
    port map(clk, a, aPipe);
    b_reg <= b when rising_edge(clk);
    
    NMProc:
    process(clk)
    begin
        if rising_edge(clk) then
            -- Find the first invalid input in a to route the b array
            -- to that position
            for i in 0 to 15 loop
                if not a(i).DataValid then
                    N <= i;
                    exit;
                end if;
            end loop;
            -- edge condition (all of a was valid)
            if a(15).DataValid then
                N <= 16;
            end if;
        end if;
    end process;
    
    -- Compute an address for every input
    -- Also clock the input into the next array to keep sync with addr
    OLoop:
    for i in 0 to 3 generate
        ILoop:
        for j in 0 to 3 generate
            signal k0  : integer := 0;
            signal k1  : integer := 0;
            signal k2  : integer := 0;
            signal ki0 : Int.DataType.tData := (0, True, True);
            signal ki1 : Int.DataType.tData := (0, True, True);
            signal ki2 : Int.DataType.tData := (0, True, True);
        begin
            k0 <= N + 4 * i + j;
            -- Slice the lowest 2 bits. Aka x % 4
            k1 <= to_integer(to_unsigned(k0, 6)(1 downto 0));
            -- Slice the next 2 bits. Aka (x % 16) // 4
            k2 <= to_integer(to_unsigned(XA1(i)(j).x, 6)(3 downto 2));
            ki0.x <= k0;
            ki1.x <= k1;
            ki2.x <= k2;
            X0(i)(j) <= b_reg(4*i + j);
            XA0(i)(j) <= ki0; 
            XLA0(i)(j) <= ki1;
            XLA1(i)(j) <= ki2; 

            -- Inter layer connections
            X1(i)(j) <= Y0(j)(i);
            XA1(i)(j) <= YA0(j)(i);
            X2(i)(j) <= Y1(j)(i);
            XA2(i)(j) <= YA1(j)(i);
        end generate;

        -- First route layer
        Route0:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X0(i),
            DataInGlobAddr  => XA0(i),
            Addr            => XLA0(i),
            DataOut         => Y0(i),
            DataOutGlobAddr => YA0(i)
        );

        -- Second route layer
        Route1:
        entity work.UniqueRouter
        port map(
            clk             => clk,
            DataIn          => X1(i),
            DataInGlobAddr  => XA1(i),
            Addr            => XLA1(i),
            DataOut         => Y1(i),
            DataOutGlobAddr => YA1(i)
        );
    end generate;

    -- Fan out the 16 to 32
    bRoute:
    for i in 0 to 31 generate
        bRouteProc:
        process(clk)
        begin
            if rising_edge(clk) then
                if XA2((i mod 16) / 4)(i mod 4).x = i and X2((i mod 16) / 4)(i mod 4).DataValid then
                    bRouted(i) <= X2((i mod 16) / 4)(i mod 4); 
                --elsif not X2((i mod 4) / 4)(i mod 2).DataValid then
                else
                    bRouted(i) <= cNull;
                    if X2((i mod 16) / 4)(i mod 4).FrameValid then
                        bRouted(i).FrameValid <= True;
                    end if;
                end if;
            end if;
        end process;
    end generate;

    FinalRoute:
    for i in 0 to 31 generate
        FRLowerHalf:
        if i < 16 generate
            FinalRouteProcA:
            process(clk)
            begin
                if rising_edge(clk) then
                    if aPipe(RouterLatency-1)(i).DataValid then
                        Y(i) <= aPipe(RouterLatency-1)(i);
                    elsif bRouted(i).DataValid then
                        Y(i) <=  bRouted(i);
                    else
                        Y(i) <= cNull;
                        if aPipe(RouterLatency-1)(i).FrameValid then
                            Y(i).FrameValid <= True;
                        end if;
                    end if;
                end if;
            end process;
        end generate;
        FRUpperHalf:
        if i >= 16 generate
            FinalRouteProcB:
            process(clk)
            begin
                if rising_edge(clk) then
                    if bRouted(i).DataValid then
                        Y(i) <= bRouted(i);
                    else
                        Y(i) <= cNull;
                        if bRouted(i).FrameValid then
                            Y(i).FrameValid <= True;
                        end if;
                    end if;
                end if;
            end process;
        end generate;
    end generate;

    ProcOut:
    process(clk)
    begin
        if rising_edge(clk) then
            q <= Y;
        end if;
    end process;

end rtl;

